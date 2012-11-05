#include "StreamBlob.h"

#include "../utilities/Utils.h"

#include <limits>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QVector>

StreamBlob::StreamBlob()
  : DataBlob("StreamBlob")
{
  mWidth = mHeight = 512;

  mXXReal.resize(288 * 288, 0.0f);
  mXXImag.resize(288 * 288, 0.0f);
  mYYReal.resize(288 * 288, 0.0f);
  mYYImag.resize(288 * 288, 0.0f);
  mXYReal.resize(288 * 288, 0.0f);
  mXYImag.resize(288 * 288, 0.0f);
  mYXReal.resize(288 * 288, 0.0f);
  mYXImag.resize(288 * 288, 0.0f);

  mSkyMap.resize(mWidth * mHeight);
  mVisMap.resize(mWidth * mHeight);
  mFrequency = 0.0;
  reset();
}

void StreamBlob::reset()
{
  mMJDTime = -1.0;
  mChannelId = 0;
}

void StreamBlob::serialise(QIODevice &out) const
{
  QDataStream stream(&out);

  stream << mDateTime;
  stream << mWidth;
  stream << mHeight;

  for (int i = 0, n = mSkyMap.size(); i < n; i++)
    stream << mSkyMap[i];
}

void StreamBlob::deserialise(QIODevice &in, QSysInfo::Endian)
{
  QDataStream stream(&in);

  stream >> mDateTime;
  stream >> mWidth;
  stream >> mHeight;

  mSkyMap.resize(mWidth * mHeight);

  for (int i = 0, n = mWidth * mHeight; i < n; i++)
    stream >> mSkyMap[i];
}

void StreamBlob::createImage(const std::vector<unsigned char> &inData, const QString &inType)
{
  static QVector<QRgb> colors;

  if (colors.empty())
  {
    QColor color;

    for (int i = 0; i < 256; i++)
    {
      //      int h = (int) round((i/256.0)*360.0);
      //      color.setHsv(h, 100, 100);
      color.setRgb(i, i, i);
      colors.append(color.rgb());
    }
  }

  QImage image(&inData[0], 512, 512, QImage::Format_Indexed8);
  image.setColorTable(colors);
  QString filename = inType + mDateTime.toString("_dd-MM-yyyy_hh-mm-ss") + ".tiff";
  image.save(filename, "TIFF");
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
  mXXReal[inA1 * 288 + inA2] = conj.real();
  mXXReal[inA2 * 288 + inA1] = inXX.real();
  mXXImag[inA1 * 288 + inA2] = conj.imag();
  mXXImag[inA2 * 288 + inA1] = inXX.imag();
  conj = std::conj<float>(inYY);
  mYYReal[inA1 * 288 + inA2] = conj.real();
  mYYReal[inA2 * 288 + inA1] = inYY.real();
  mYYImag[inA1 * 288 + inA2] = conj.imag();
  mYYImag[inA2 * 288 + inA1] = inYY.imag();
  conj = std::conj<float>(inXY);
  mXYReal[inA1 * 288 + inA2] = conj.real();
  mXYReal[inA2 * 288 + inA1] = inXY.real();
  mXYImag[inA1 * 288 + inA2] = conj.imag();
  mXYImag[inA2 * 288 + inA1] = inXY.imag();
  conj = std::conj<float>(inYX);
  mYXReal[inA1 * 288 + inA2] = conj.real();
  mYXReal[inA2 * 288 + inA1] = inYX.real();
  mYXImag[inA1 * 288 + inA2] = conj.imag();
  mYXImag[inA2 * 288 + inA1] = inYX.imag();
}

std::vector<float>* StreamBlob::getXXReal()
{
  return &mXXReal;
}

std::vector<float>* StreamBlob::getXXImag()
{
  return &mXXImag;
}

const std::vector<float>* StreamBlob::getXXReal() const
{
  return &mXXReal;
}

const std::vector<float>* StreamBlob::getXXImag() const
{
  return &mXXImag;
}


void StreamBlob::setMJDTime(const double inTime)
{
  mMJDTime = inTime;
  mDateTime = utils::MJD2QDateTime(inTime);
}
