#ifndef STREAM_BLOB_H
#define STREAM_BLOB_H

#include "../Constants.h"
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
  Eigen::MatrixXcf mData[MAX_MERGE_CHANNELS][NUM_POLARIZATIONS]; // visibilities
  Eigen::MatrixXf mMasks[MAX_MERGE_CHANNELS][NUM_POLARIZATIONS]; // 1.0f is flagged
  Eigen::MatrixXcf mVisibilities;
  std::vector<int> mFlagged[MAX_MERGE_CHANNELS][NUM_POLARIZATIONS]; // flagged ants

  Eigen::MatrixXf mSkyMap;
  int mImageWidth;
  int mImageHeight;
  int mNumChannels;

  /// Reset the blob for reuse
  void reset();

  /// Add visibilities for all polarizations (xx,yy,xy,yx)
  void addVis(const quint16 channel,
              const quint16 a1,
              const quint16 a2,
              const std::complex<float> v[]);

  /// Prepare for sending
  void serialise(QIODevice &out) const;

  /// Receive blob
  void deserialise(QIODevice &in, QSysInfo::Endian);

  /// Compute raw statistics
  void computeStats();
};

PELICAN_DECLARE_DATABLOB(StreamBlob)

#endif // STREAM_BLOB_H

