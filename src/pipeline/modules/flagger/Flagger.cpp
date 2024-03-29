#include "Flagger.h"

#include "../../StreamBlob.h"
#include "Constants.h"
#include "../../../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <algorithm>
#include <QtCore>

Flagger::Flagger(const ConfigNode &inConfig):
 AbstractModule(inConfig)
{
  mAntennas.resize(NUM_ANTENNAS);
  mAntTmp.resize(NUM_ANTENNAS);
  mAntSigma = inConfig.getOption("antenna", "sigma", "3").toFloat();
  mVisSigma = inConfig.getOption("visibility", "sigma", "2").toFloat();
  QString s = inConfig.getOption("antenna", "flagged", "");
  mFlaggedAnts = ParseFlagged(s);
  for (int i = 0, n = mFlaggedAnts.size(); i < n; i++)
    qWarning("Flagged from configuration: %i", mFlaggedAnts[i]);
  qDebug("Antennas     Nof Sigmas: %0.1f", mAntSigma);
  qDebug("Visibilities Nof Sigmas: %0.1f", mVisSigma);
}

Flagger::~Flagger()
{
}

std::vector<int> Flagger::ParseFlagged(const QString &s)
{
  std::vector<int> flagged;
  if (s.isEmpty())
    return flagged;

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
  flagged.push_back(antenna);

  return flagged;
}

void Flagger::run(const int pol, const StreamBlob *input, StreamBlob *output)
{
  using namespace std;
  const int N = NUM_BASELINES;
  const int M = input->mNumChannels;
  const int HALF_M = M / 2;

  mAbs.resize(M, N);
  mTmp.resize(M, N);
  mMask.resize(M, N);
  mStd.resize(N);
  mCentroid.resize(N);
  mMean.resize(N);
  mMinVal.resize(N);
  mMaxVal.resize(N);
  mResult.resize(N);

  // Compute statistics only when we have enough channels
  if (M >= 3)
  {
    // Compute power of visibilities
    mAbs = input->mRawData[pol].array().abs();
    mTmp = mAbs;

    // computes the exact median
    if (M & 1)
    {
      for (int i = 0; i < N; i++)
      {
        vector<float> row(mTmp.data() + i * M, mTmp.data() + (i + 1) * M);
        nth_element(row.begin(), row.begin() + HALF_M, row.end());
        mCentroid(i) = row[HALF_M];
      }
    }
    // nth_element guarantees x_0,...,x_{n-1} < x_n
    else
    {
      for (int i = 0; i < N; i++)
      {
        vector<float> row(mTmp.data() + i * M, mTmp.data() + (i + 1) * M);
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
    mStd *= mVisSigma;
    mMinVal = mCentroid - mStd;
    mMaxVal = mCentroid + mStd;

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
  }
  else
  {
    mResult = output->mRawData[pol].colwise().mean();
  }

  // construct acm from result
  for (int i = 0, s = 0; i < NUM_ANTENNAS; i++)
  {
    output->mCleanData[pol].col(i).head(i + 1) = mResult.segment(s, i + 1).conjugate();
    output->mCleanData[pol].row(i).head(i + 1) = mResult.segment(s, i + 1);
    output->mCleanData[pol](i,i).imag() = 0.0f;
    s += i + 1;
  }

  // clear NaN values
  output->mCleanData[pol] =
      (output->mCleanData[pol].array() != output->mCleanData[pol].array()).select(complex<float>(0.0f, 0.0f), output->mCleanData[pol]);

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

  // Create "dipoles" by computing the absolute mean of the visibilities
  mAntennas = output->mCleanData[pol].array().abs().colwise().mean();
  for (int a = 0; a < mAntennas.size(); a++)
  {
    if (mAntennas(a) <= 1e-5f && 
        std::find(output->mFlagged[pol].begin(), output->mFlagged[pol].end(), a)
        == output->mFlagged[pol].end())
    {
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
    }
  }

  // Sigma clip the remaining dipoles
  mAntTmp = mAntennas;
  float mean = mAntennas.mean();
  float std = sqrtf((1.0f / mAntennas.size()) *
                    (mAntennas.array() - mean).array().square().sum());
  float centroid;
  vector<float> ant(mAntTmp.data(), mAntTmp.data()+mAntTmp.size());
  nth_element(ant.begin(), ant.begin() + ant.size()/2, ant.end());
  centroid = ant[ant.size()/2];
  centroid += *max_element(ant.begin(), ant.begin() + ant.size()/2);
  centroid *= 0.5f;

  // Now we can determine bad antennas
  std *= mAntSigma;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (centroid - std) || mAntennas(a) > (centroid + std))
    {
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
    }
  }
  qDebug("[%s] Flagged %li dipoles, %i visibilities", (pol ? "YY" : "XX"),
      output->mFlagged[pol].size(), M >= 3 ? (int(mMask.size() - mMask.sum())) : 0);
}

