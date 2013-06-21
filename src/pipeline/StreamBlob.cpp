#include "StreamBlob.h"

#include <limits>

StreamBlob::StreamBlob():
  DataBlob("StreamBlob"),
  mImageWidth(IMAGE_OUTPUT_SIZE),
  mImageHeight(IMAGE_OUTPUT_SIZE),
  mNumChannels(0)
{
  // Initialize eigen matrices
  for (int c = 0; c < MAX_MERGE_CHANNELS; c++)
  {
    for (int p = 0; p < NUM_POLARIZATIONS; p++)
    {
      mData[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
      mMasks[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
    }
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

  for (int c = 0; c < mNumChannels; c++)
  {
    mData[c][XX_POL].setZero();
    mFlagged[c][XX_POL].clear();
    mMasks[c][XX_POL].setIdentity();
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

void StreamBlob::addVis(const quint16 channel,
                        const quint16 a1,
                        const quint16 a2,
                        const std::complex<float> v[])
{
  for (int p = 0; p < NUM_POLARIZATIONS; p++)
  {
    mData[channel-mHeader.start_chan][p](a1,a2) = v[p];
    mData[channel-mHeader.start_chan][p](a2,a1) = std::conj(v[p]);
  }
}
