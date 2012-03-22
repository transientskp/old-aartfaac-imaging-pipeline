#include "UniboardDataBlob.h"

#include "../utilities/Utils.h"

#include <limits>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QVector>

UniboardDataBlob::UniboardDataBlob()
  : DataBlob("UniboardDataBlob")
{
  mWidth = mHeight = 512;

  mXXReal.resize(288*288, 0.0f);
  mXXImag.resize(288*288, 0.0f);
  mSkyMap.resize(mWidth * mHeight);
  mVisMap.resize(mWidth * mHeight);
  reset();
}

void UniboardDataBlob::reset()
{
  mMJDTime = -1.0;
  mChannelId = 0;
}

void UniboardDataBlob::serialise(QIODevice &out) const
{
  QDataStream stream(&out);

  stream << mWidth;
  stream << mHeight;

  for (int i = 0, n = mSkyMap.size(); i < n; i++)
    stream << mSkyMap[i];
}

void UniboardDataBlob::deserialise(QIODevice &in, QSysInfo::Endian)
{
  QDataStream stream(&in);

  stream >> mWidth;
  stream >> mHeight;

  mSkyMap.resize(mWidth * mHeight);

  for (int i = 0, n = mWidth*mHeight; i < n; i++)
    stream >> mSkyMap[i];
}

void UniboardDataBlob::createImage(const std::vector<unsigned char> &inData, const QString &inType)
{
  static QVector<QRgb> colors;
  if (colors.empty())
  {
    QColor color;
    for (int i = 0; i < 256; i++)
    {
//      int h = (int) round((i/256.0)*360.0);
//      color.setHsv(h, 100, 100);
      color.setRgb(i,i,i);
      colors.append(color.rgb());
    }
  }

  QImage image(&inData[0], 512, 512, QImage::Format_Indexed8);
  image.setColorTable(colors);
  QString filename = inType + mDateTime.toString("_dd-MM-yyyy_hh-mm-ss") + ".tiff";
  image.save(filename, "TIFF");
}

void UniboardDataBlob::addSample(const quint16 inA1,
                                 const quint16 inA2,
                                 const std::complex<float> &inXX,
                                 const std::complex<float> &inYY,
                                 const std::complex<float> &inXY,
                                 const std::complex<float> &inYX)
{
  Q_UNUSED(inYY);
  Q_UNUSED(inXY);
  Q_UNUSED(inYX);

  std::complex<float> xx_conj = std::conj<float>(inXX);
  mXXReal[inA1*288+inA2] = inXX.real();
  mXXReal[inA2*288+inA1] = xx_conj.real();
  mXXImag[inA1*288+inA2] = inXX.imag();
  mXXImag[inA2*288+inA1] = xx_conj.imag();
}

const std::vector<float>* UniboardDataBlob::getXXReal() const
{
  return &mXXReal;
}

const std::vector<float>* UniboardDataBlob::getXXImag() const
{
  return &mXXImag;
}


void UniboardDataBlob::setMJDTime(const double inTime)
{
  mMJDTime = inTime;
  mDateTime = utils::MJD2QDateTime(inTime);
}
