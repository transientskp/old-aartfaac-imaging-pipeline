#include "StreamBlob.h"
#include "../utilities/Utils.h"
#include "../utilities/AntennaPositions.h"

#include <limits>

StreamBlob::StreamBlob():
  DataBlob("StreamBlob"),
  mImageWidth(IMAGE_OUTPUT_SIZE),
  mImageHeight(IMAGE_OUTPUT_SIZE),
  mNumChannels(0),
  mDl(0.0f)
{
  // Initialize eigen matrices
  for (int p = 0; p < NUM_USED_POLARIZATIONS; p++)
  {
    mCleanData[p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
    mMasks[p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
    mFlagged[p].clear();
  }

  mHeader.start_chan = 0;
  mHeader.end_chan = 0;
  mSkyMap.resize(mImageWidth, mImageHeight);

  reset();
}

void StreamBlob::reset()
{
  mNumChannels = mHeader.end_chan - mHeader.start_chan + 1;
  if (mNumChannels > MAX_MERGE_CHANNELS)
    qFatal("Too many channels: %d <= %d does not hold", mNumChannels, MAX_MERGE_CHANNELS);

  for (int p = 0; p < NUM_USED_POLARIZATIONS; p++)
  {
    mRawData[p].resize(mNumChannels, NUM_BASELINES);
    mCleanData[p].setZero();
    mMasks[p].setIdentity();
    mFlagged[p].clear();
  }

  mSkyMap.setZero();
}

void StreamBlob::serialise(QIODevice &out) const
{
  Q_UNUSED(out);
}

void StreamBlob::deserialise(QIODevice &in, QSysInfo::Endian)
{
  Q_UNUSED(in);
}

float StreamBlob::centralFreq() const
{
  return utils::Range2Frequency(mHeader.subband, mHeader.start_chan, mHeader.end_chan);
}

float StreamBlob::bandWidth() const
{
  return utils::Channel2Frequency(mHeader.subband, mHeader.end_chan) - utils::Channel2Frequency(mHeader.subband, mHeader.start_chan);
}

