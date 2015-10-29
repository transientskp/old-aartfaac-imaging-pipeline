#include "Flagger.h"

#include "../../StreamBlob.h"
#include "Constants.h"
#include "../../../utilities/Utils.h"
#include "../../../utilities/monitoring/Server.h"

#include <pelican/utility/Config.h>
#include <algorithm>
#include <QtCore>

Flagger::Flagger(const ConfigNode &inConfig):
 AbstractModule(inConfig)
{
  mAntennas.resize(NUM_ANTENNAS);
  mAntSigma = inConfig.getOption("antenna", "sigma", "4").toFloat();
  mVisSigma = inConfig.getOption("visibility", "sigma", "2").toFloat();
  QString s = inConfig.getOption("antenna", "flagged", "");
  mFlaggedAnts = ParseFlagged(s);
  for (int i = 0, n = mFlaggedAnts.size(); i < n; i++)
    qWarning("Flagged from configuration: %i", mFlaggedAnts[i]);
}

Flagger::~Flagger()
{
}

std::vector<int> Flagger::ParseFlagged(const QString &s)
{
  std::vector<int> flagged;
  int antenna = -1;
  bool success;

  for (int i = 0; i < s.size(); )
  {
    QChar c = s.at(i);
    if (c.isDigit())
    {
      int j = i + 1;
      while (j < s.size() && s[j].isDigit())
        j++;

      antenna = s.mid(i,j-i).toInt(&success);
      if (!success)
        qFatal("Invalid number %s", qPrintable(s.mid(i,j-i)));
      i += j - i;
    }
    else
    {
      switch (c.toAscii())
      {
      case ',':
        flagged.push_back(antenna);
        break;
      default: qFatal("Invalid character `%c' at line %d", s[i].toAscii(), i);
      }
      i++;
    }
  }

  return flagged;
}

void Flagger::run(const int pol, const StreamBlob *input, StreamBlob *output)
{
  const int N = NUM_BASELINES;
  const int M = input->mNumChannels;
  const int HALF_M = M / 2;

  mAbs.resize(M, N);
  mMask.resize(M, N);
  mStd.resize(N);
  mCentroid.resize(N);
  mMean.resize(N);
  mMinVal.resize(N);
  mMaxVal.resize(N);
  mResult.resize(N);

  // Compute power of visibilities
  mAbs = input->mRawData[pol].array().abs();

  // computes the exact median
  if (M & 1)
  {
    for (int i = 0; i < N; i++)
    {
      vector<float> row(mAbs.data() + i * M, mAbs.data() + (i + 1) * M);
      nth_element(row.begin(), row.begin() + HALF_M, row.end());
      mCentroid(i) = row[HALF_M];
    }
  }
  // nth_element guarantees x_0,...,x_{n-1} < x_n
  else
  {
    for (int i = 0; i < N; i++)
    {
      vector<float> row(mAbs.data() + i * M, mAbs.data() + (i + 1) * M);
      nth_element(row.begin(), row.begin() + HALF_M, row.end());
      mCentroid(i) = row[HALF_M];
      mCentroid(i) += *max_element(row.begin(), row.begin() + HALF_M);
      mCentroid(i) *= 0.5f;
    }
  }

  // compute the mean
  mMean = mAbs.colwise().mean();

  // compute std (x) = sqrt ( 1/M SUM_i (x(i) - mean(x))^2 )
  mStd =
      (((mAbs.rowwise() - mMean.transpose()).array().square()).colwise().sum() *
       (1.0f / M))
          .array()
          .sqrt();

  // compute n sigmas from centroid
  mMinVal = mCentroid - mStd * mVisSigma;
  mMaxVal = mCentroid + mStd * mVisSigma;

  // compute clip mask
  for (int i = 0; i < N; i++)
  {
    mMask.col(i) =
        (mAbs.col(i).array() > mMinVal(i)).select(VectorXf::Ones(M), 0.0f);
    mMask.col(i) =
        (mAbs.col(i).array() < mMaxVal(i)).select(VectorXf::Ones(M), 0.0f);
  }

  // apply clip mask to data
  output->mRawData[pol].array() *= mMask.array();

  // compute mean such that we ignore clipped data, this is our final result
  mResult = output->mRawData[pol].colwise().sum().array() /
            mMask.colwise().sum().array();

  // construct acm from result
  for (int i = 0, s = 0; i < NUM_ANTENNAS; i++)
  {
    output->mCleanData[pol].col(i).head(i + 1) =
        mResult.segment(s, i + 1).conjugate();
    output->mCleanData[pol].row(i).head(i + 1) = mResult.segment(s, i + 1);
    s += i + 1;
  }

  // Flag on antenna/dipole level, from xml file
  if (!mFlaggedAnts.empty())
  {
    for (int i = 0, n = mFlaggedAnts.size(); i < n; i++)
    {
      int a = mFlaggedAnts[i];
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
    }
  }

  // clear NaN values
  output->mCleanData[pol] =
      (output->mCleanData[pol].array() != output->mCleanData[pol].array())
          .select(complex<float>(0.0f, 0.0f), output->mCleanData[pol]);

  // Flag antennas using sigma clipping
  mAntennas = output->mCleanData[pol].array().abs().colwise().mean();
  float mean = mAntennas.mean();
  float std = sqrtf((1.0f / mAntennas.size()) *
                    (mAntennas.array() - mean).array().square().sum());

  // Now we can determine bad antennas
  std *= mAntSigma;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (mean - std) || mAntennas(a) > (mean + std))
    {
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
    }
  }
  qDebug("[%s] Flagged %li dipoles, %i visibilities", (pol ? "YY" : "XX"),
         output->mFlagged[pol].size(), int(mMask.size() - mMask.sum()));
}

