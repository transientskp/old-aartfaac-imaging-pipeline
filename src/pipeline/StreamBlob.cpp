#include "StreamBlob.h"

#include "../utilities/Utils.h"
#include "../Constants.h"

#include <limits>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QVector>

StreamBlob::StreamBlob()
  : DataBlob("StreamBlob")
{
  mWidth = mHeight = IMAGE_OUTPUT_SIZE;

  mXX.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mYY.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mXY.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mYX.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  mSkyMap.resize(mHeight, mWidth);
  mFlagged.resize(NUM_ANTENNAS, false);

  reset();
}

void StreamBlob::reset()
{
  mMJDTime = -1.0;
  mChannelId = 0;
  mFrequency = 0.0f;
  mFlagged.assign(NUM_ANTENNAS, false);
  mSkyMap = Eigen::MatrixXf::Zero(mHeight, mWidth);
  mXX = mYY = mXY = mYX = Eigen::MatrixXcf::Zero(NUM_ANTENNAS, NUM_ANTENNAS);
}

void StreamBlob::serialise(QIODevice &out) const
{
  QDataStream stream(&out);

  stream << mDateTime;
  stream << mWidth;
  stream << mHeight;

  for (unsigned int i = 0; i < mHeight; i++)
    for (unsigned int j = 0; j < mWidth; j++)
      stream << mSkyMap(i,j);
}

void StreamBlob::deserialise(QIODevice &in, QSysInfo::Endian)
{
  QDataStream stream(&in);

  stream >> mDateTime;
  stream >> mWidth;
  stream >> mHeight;

  if (mSkyMap.rows() != mHeight || mSkyMap.cols() != mWidth)
    mSkyMap.resize(mHeight, mWidth);

  for (unsigned int i = 0; i < mHeight; i++)
    for (unsigned int j = 0; j < mWidth; j++)
      stream >> mSkyMap(i,j);
}

void StreamBlob::addSample(const quint16 inA1,
                           const quint16 inA2,
                           const std::complex<float> &inXX,
                           const std::complex<float> &inYY,
                           const std::complex<float> &inXY,
                           const std::complex<float> &inYX)
{
  std::complex<float> conj;

  conj = std::conj<float>(inXX);
  mXX(inA1, inA2) = inXX;
  mXX(inA2, inA1) = conj;
  conj = std::conj<float>(inYY);
  mYY(inA1, inA2) = inYY;
  mYY(inA2, inA1) = conj;
  conj = std::conj<float>(inXY);
  mXY(inA1, inA2) = inXY;
  mXY(inA2, inA1) = conj;
  conj = std::conj<float>(inYX);
  mYX(inA1, inA2) = inYX;
  mYX(inA2, inA1) = conj;
}

void StreamBlob::setMJDTime(const double inTime)
{
  mMJDTime = inTime;
  mDateTime = utils::MJD2QDateTime(inTime);
}
