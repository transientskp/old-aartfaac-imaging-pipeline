#include "StreamBlob.h"
#include "../utilities/Utils.h"
#include "../utilities/monitoring/Server.h"
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
  for (int p = 0; p < NUM_POLARIZATIONS; p++)
  {
    mCleanData[p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
    mMasks[p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
    mFlagged[p].clear();
  }

  for (int c = 0; c < MAX_MERGE_CHANNELS; c++)
    for (int p = 0; p < NUM_POLARIZATIONS; p++)
      mRawData[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);

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

  for (int p = 0; p < NUM_POLARIZATIONS; p++)
  {
    mCleanData[p].setZero();
    mFlagged[p].clear();
    mMasks[p].setIdentity();
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

void StreamBlob::computeStats()
{
  static const double distance = 320.0;
  ADD_STAT("FNORM", mHeader.time, mCleanData[XX_POL].norm());

  std::complex<float> sum;
  int count = 0;
  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    Eigen::RowVector3d pos_a1 = ANT_XYZ(a1);
    for (int a2 = a1 + 1; a2 < NUM_ANTENNAS; a2++)
    {
      Eigen::RowVector3d pos_a2 = ANT_XYZ(a2);
      if ((pos_a1 - pos_a2).norm() > distance)
      {
        sum += mCleanData[XX_POL](a1, a2);
        count++;
      }
    }
  }
  sum /= count;
  // add fringe amplitude and phase
  ADD_STAT("FRINGE_AMPLITUDE", mHeader.time, std::abs(sum));
  ADD_STAT("FRINGE_PHASE", mHeader.time, std::atan2(sum.imag(), sum.real()));
}

float StreamBlob::centralFreq() const
{
  return mHeader.freq + (mHeader.end_chan - mHeader.start_chan)*mHeader.chan_width;
}

void StreamBlob::addVis(const int channel,
                        const quint16 a1,
                        const quint16 a2,
                        std::complex<float> v[])
{
  for (int p = 0; p < NUM_POLARIZATIONS; p++)
  {
    mRawData[channel][p](a2, a1) = v[p];
    mRawData[channel][p](a1, a2) = std::conj(v[p]);
    v[p] /= mNumChannels;
    mCleanData[p](a2, a1) += v[p];
    mCleanData[p](a1, a2) += std::conj(v[p]);
  }
}
