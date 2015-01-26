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
  mAntennas.resize(NUM_ANTENNAS);
  mSortedAntennas.resize(NUM_ANTENNAS);
  mNumSigmas = inConfig.getOption("deviation", "multiplier", "4").toFloat();
}

Flagger::~Flagger()
{
}

void Flagger::run(const int pol, const StreamBlob *input, StreamBlob *output)
{
  // Compute the amplitude of each complex value
  mAmplitudes = input->mData[pol].cwiseAbs();

  // Compute the mean per antenna without autocorrelations
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    mAmplitudes(a,a) = 0.0f;
    mAntennas(a) = mAmplitudes.row(a).mean();
    mSortedAntennas[a] = mAntennas(a);
  }

  std::sort(mSortedAntennas.begin(), mSortedAntennas.end());

  // Compute the median, mean and variance
  float median = (mSortedAntennas[NUM_ANTENNAS/2] + mSortedAntennas[NUM_ANTENNAS/2 - 1]) / 2.0f;
  float mean = mAntennas.mean();
  float variance = 0.0f;
  float probability = 1.0f / NUM_ANTENNAS;

  for (int a = 0; a < NUM_ANTENNAS; a++)
    variance += probability * ((mAntennas(a) - mean) * (mAntennas(a) - mean));

  // Now we can determine bad antennas
  float std = sqrtf(variance) * mNumSigmas;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (median - std) || mAntennas(a) > (median + std))
    {
      qWarning("Antenna %d is bad, flagged", a);
      output->mMasks[pol].col(a).setOnes();
      output->mMasks[pol].row(a).setOnes();
      output->mFlagged[pol].push_back(a);
      ADD_STAT("FLAGGER_" << a, input->mHeader.time, mAntennas(a));
    }
  }

  // Flag individual visibilities
  float sum = 0.0f;
  int r = output->mFlagged[pol].size();
  //      all ants                  - removed rows and cols      - diagonal
  int n = NUM_ANTENNAS*NUM_ANTENNAS - r*(NUM_ANTENNAS-r)*2 + r*r - (NUM_ANTENNAS-r);
  for (int i = 0; i < NUM_ANTENNAS; i++)
    for (int j = 0; j < NUM_ANTENNAS; j++)
      sum += mAmplitudes(i, j) * (1.0f - output->mMasks[pol](i, j));

  // compute variance
  mean = sum / n;
  variance = 0.0f;
  probability = 1.0f / n;
  for (int i = 0; i < NUM_ANTENNAS; i++)
    for (int j = 0; j < NUM_ANTENNAS; j++)
      variance += probability * (mAmplitudes(i, j)-mean) * (mAmplitudes(i, j)-mean) * (1.0f - output->mMasks[pol](i, j));

  // apply masking of abs(visibilities) outside mean +/- std*n
  std = sqrtf(variance) * mNumSigmas;
  for (int i = 0; i < NUM_ANTENNAS; i++)
    for (int j = 0; j < NUM_ANTENNAS; j++)
      if (mAmplitudes(i, j) < (mean - std) || mAmplitudes(i, j) > (mean + std))
        output->mMasks[pol](i, j) = 1.0f;
  qWarning("Individuals flagged: %i", int(output->mMasks[pol].sum())-(NUM_ANTENNAS-r)-r*(NUM_ANTENNAS-r)*2 + r*r);
}

