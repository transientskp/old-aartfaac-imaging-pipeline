#include "Flagger.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"

#include <pelican/utility/Config.h>
#include <algorithm>
#include <QtCore>

Flagger::Flagger(const ConfigNode &inConfig):
 AbstractModule(inConfig)
{
  mAmplitudes.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mAntennas.resize(NUM_ANTENNAS);
  mSortedAntennas.resize(NUM_ANTENNAS);
  mDeviationMultiplier = inConfig.getOption("deviation", "multiplier", "4").toFloat();
}

Flagger::~Flagger()
{
}

void Flagger::run(const int channel, const StreamBlob *input, StreamBlob *output)
{
  // Compute the amplitude of each complex value
  mAmplitudes = input->mData[channel][XX_POL].cwiseAbs();

  // Compute the mean per antenna without autocorrelations
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    mAmplitudes(a,a) = 0.0f;
    mAntennas(a) = mAmplitudes.row(a).sum() / (NUM_ANTENNAS-1);
    mSortedAntennas[a] = mAntennas(a);
  }

  std::sort(mSortedAntennas.begin(), mSortedAntennas.end());

  // Compute the median, mean and variance
  float median = (mSortedAntennas[NUM_ANTENNAS/2] + mSortedAntennas[NUM_ANTENNAS/2 - 1]) / 2.0f;
  float mean = mAntennas.sum() / NUM_ANTENNAS;
  float variance = 0.0f;
  float probability = 1.0f / NUM_ANTENNAS;

  for (int a = 0; a < NUM_ANTENNAS; a++)
    variance += probability * ((mAntennas(a) - mean) * (mAntennas(a) - mean));

  // Now we can determine bad antennas
  float threshold = sqrtf(variance) * mDeviationMultiplier;
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    if (mAntennas(a) < (median - threshold) || mAntennas(a) > (median + threshold))
    {
      qWarning("Antenna %d is bad, flagged", a);
      output->mMasks[channel][XX_POL].col(a).setOnes();
      output->mMasks[channel][XX_POL].row(a).setOnes();
      output->mFlagged[channel][XX_POL].push_back(a);
    }
  }
}

