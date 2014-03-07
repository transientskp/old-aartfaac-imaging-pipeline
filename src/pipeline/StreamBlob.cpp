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
  for (int c = 0; c < MAX_MERGE_CHANNELS; c++)
  {
    for (int p = 0; p < NUM_POLARIZATIONS; p++)
    {
      mData[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
      mMasks[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);
      mFlagged[c][p].clear();
    }
  }

  mHeader.start_chan = 0;
  mHeader.end_chan = 0;
  mSkyMap.resize(mImageWidth, mImageHeight);
  mVisibilities.resize(NUM_ANTENNAS, NUM_ANTENNAS);

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
  mVisibilities.setZero();
  mHasConverged.resize(mNumChannels, true);
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
  mVisibilities.array() /= mNumChannels;
  ADD_STAT("FNORM", mHeader.time, mVisibilities.norm());

  std::complex<float> sum;
  int count = 0;
  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    Eigen::RowVector3d pos_a1 = ap.GetPosLocal(a1);
    for (int a2 = a1 + 1; a2 < NUM_ANTENNAS; a2++)
    {
      Eigen::RowVector3d pos_a2 = ap.GetPosLocal(a2);
      if ((pos_a1 - pos_a2).norm() > distance)
      {
        sum += mVisibilities(a1, a2);
        count++;
      }
    }
  }
  sum /= count;
  // add fringe amplitude and phase
  ADD_STAT("FRINGE_AMPLITUDE", mHeader.time, std::abs(sum));
  ADD_STAT("FRINGE_PHASE", mHeader.time, std::atan2(sum.imag(), sum.real()));
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
  mVisibilities(a1, a2) += v[XX_POL];
  mVisibilities(a2, a1) += std::conj(v[XX_POL]);
}
