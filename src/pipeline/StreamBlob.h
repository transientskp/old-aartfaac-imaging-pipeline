#ifndef STREAM_BLOB_H
#define STREAM_BLOB_H

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

  void reset();

  void addSample(const quint16 inA1,
                 const quint16 inA2,
                 const std::complex<float> &inXX,
                 const std::complex<float> &inYY,
                 const std::complex<float> &inXY,
                 const std::complex<float> &inYX);

  void serialise(QIODevice &out) const;

  void deserialise(QIODevice &in, QSysInfo::Endian);

  void setMJDTime(const double inTime);

  Eigen::MatrixXcf mXX;
  Eigen::MatrixXcf mYY;
  Eigen::MatrixXcf mXY;
  Eigen::MatrixXcf mYX;

  Eigen::MatrixXf mSkyMap;

  std::vector<int> mFlagged;

  double mMJDTime;
  double mFrequency;
  QDateTime mDateTime;
  quint32 mChannelId;
  quint32 mWidth;
  quint32 mHeight;
};

PELICAN_DECLARE_DATABLOB(StreamBlob)

#endif // STREAM_BLOB_H

