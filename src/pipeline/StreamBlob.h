#ifndef STREAM_BLOB_H
#define STREAM_BLOB_H

#include "Constants.h"
#include "../emulator/stream/StreamPacket.h"
#include <pelican/data/DataBlob.h>
#include <eigen3/Eigen/Dense>
#include <complex>
#include <vector>
#include <QtCore>

using namespace pelican;

class StreamBlob : public DataBlob
{
public:
  StreamBlob();

  ChunkHeader mHeader;
  Eigen::MatrixXcf mRawData[NUM_USED_POLARIZATIONS];   // numchannels x numbaselines
  Eigen::MatrixXcf mCleanData[NUM_USED_POLARIZATIONS]; // cleaned up array correlation matrix
  Eigen::MatrixXf mMasks[NUM_USED_POLARIZATIONS];      // 1.0f is flagged
  std::vector<int> mFlagged[NUM_USED_POLARIZATIONS];   // flagged ants

  Eigen::MatrixXf mSkyMap;
  int mImageWidth;
  int mImageHeight;
  int mNumChannels;
  float mDl;

  /// Reset the blob for reuse
  void reset();

  /// Prepare for sending
  void serialise(QIODevice &out) const;

  /// Receive blob
  void deserialise(QIODevice &in, QSysInfo::Endian);

  /// Compute the central frequency
  float centralFreq() const;
};

PELICAN_DECLARE_DATABLOB(StreamBlob)

#endif // STREAM_BLOB_H

