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
  mAmplitudes.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMean.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mStd.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mSum.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMin.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMax.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mMeanSq.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mAntennas.resize(NUM_ANTENNAS);
  mNumSigmas = inConfig.getOption("deviation", "multiplier", "4").toFloat();
}

Flagger::~Flagger()
{
}

void Flagger::run(const int pol, const StreamBlob *input, StreamBlob *output)
{
  // 1. Flag antennas using sigma clipping on the average across channels
  mAmplitudes = input->mCleanData[pol].cwiseAbs();
  mAntennas = mAmplitudes.colwise().mean();
  float mean = mAntennas.mean();
  float std = sqrtf(1.0f/(mAntennas.size()-1) * (mAntennas.array().square() - mean*mean).sum());

  // Now we can determine bad antennas
  std *= mNumSigmas;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (mean - std) || mAntennas(a) > (mean + std))
    {
      qWarning("Antenna %d is bad, flagged", a);
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
      ADD_STAT("FLAGGER_" << a, input->mHeader.time, mAntennas(a));
    }
  }

  // 2. Filter out individual visibilities across channels using sigma clipping
  mMean.setZero();
  mStd.setZero();
  output->mCleanData[pol].setZero();
  mSum = output->mMasks[pol];

  // Compute mean and mean^2
  for (int c = 0; c < input->mNumChannels; c++)
    mMean.array() += input->mRawData[c][pol].array().abs();
  mMean.array() /= input->mNumChannels;
  mMeanSq = mMean.array().square();

  // Compute standard deviation
  for (int c = 0; c < input->mNumChannels; c++)
    mStd.array() += (input->mRawData[c][pol].array().abs().square() - mMeanSq.array());
  mStd.array() /= (input->mNumChannels-1.0f);
  mStd = mStd.array().sqrt();

  // Filter out bad visibilities
  mMin = mMean.array() - mStd.array()*mNumSigmas;
  mMax = mMean.array() + mStd.array()*mNumSigmas;
  MatrixXf good(NUM_ANTENNAS, NUM_ANTENNAS);
  for (int c = 0; c < input->mNumChannels; c++)
  {
    good = 1.0f - output->mMasks[pol].array();
    good = (input->mRawData[c][pol].array().abs() > mMin.array()).select(good, 0.0f);
    good = (input->mRawData[c][pol].array().abs() < mMax.array()).select(good, 0.0f);
    output->mCleanData[pol].array() += input->mRawData[c][pol].array() * good.array();
    mSum.array() += good.array();
  }

  // Final averaged visibilities
  output->mCleanData[pol].array() /= mSum.array();
}

