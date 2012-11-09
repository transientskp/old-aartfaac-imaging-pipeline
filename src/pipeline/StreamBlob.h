#ifndef STREAM_BLOB_H
#define STREAM_BLOB_H

#include <pelican/data/DataBlob.h>
#include <vector>
#include <complex>
#include <QtCore>

using namespace pelican;

class StreamBlob : public DataBlob
{
public:
  StreamBlob();

  std::vector<float> mXXReal; ///< xx polarized real part of the complex numbers
  std::vector<float> mXXImag; ///< xx polarized imaginary part of the complex numbers
  std::vector<float> mYYReal; ///< yy polarized real part of the complex numbers
  std::vector<float> mYYImag; ///< yy polarized imaginary part of the complex numbers
  std::vector<float> mXYReal; ///< xy polarized real part of the complex numbers
  std::vector<float> mXYImag; ///< xy polarized imaginary part of the complex numbers
  std::vector<float> mYXReal; ///< yx polarized real part of the complex numbers
  std::vector<float> mYXImag; ///< yx polarized imaginary part of the complex numbers
  std::vector<float> mSkyMap; ///< raw skymap data from imager (needs normalization)
  std::vector<float> mSkyMapradec; ///< raw skymap data from imager (needs normalization)
  std::vector<float> mVisMap; ///< raw vismap data from imager (needs normalization)

  void reset();

  void addSample(const quint16 inA1,
                 const quint16 inA2,
                 const std::complex<float> &inXX,
                 const std::complex<float> &inYY,
                 const std::complex<float> &inXY,
                 const std::complex<float> &inYX);

  void setMJDTime(const double inMJDTime);

  void serialise(QIODevice &out) const;

  void deserialise(QIODevice &in, QSysInfo::Endian);

  const std::vector<float>* getXXReal() const;

  const std::vector<float>* getXXImag() const;

  std::vector<float>* getXXReal();

  std::vector<float>* getXXImag();

  void setFrequency(const double inFrequency)
  {
    mFrequency = inFrequency;
  }
  quint32 getFrequency() const
  {
    return mFrequency;
  }
  double getMJDTime() const
  {
    return mMJDTime;
  }
  QDateTime getDateTime() const
  {
    return mDateTime;
  }
  quint32 getWidth() const
  {
    return mWidth;
  }
  quint32 getHeight() const
  {
    return mHeight;
  }

  std::vector<float>& getSkyMap()              { return mSkyMap; }
  std::vector<float>& getSkyMapradec()         { return mSkyMapradec; }
  std::vector<float>& getVisMap()              { return mVisMap; }
  const std::vector<float>& getSkyMap() const  { return mSkyMap; }
  const std::vector<float>& getSkyMapradec() const  { return mSkyMapradec; }
  const std::vector<float>& getVisMap() const  { return mVisMap; }

  void createImage(const std::vector<unsigned char> &inData, const QString &inType);

private:
  double mMJDTime;
  double mFrequency;
  QDateTime mDateTime;
  quint32 mChannelId;
  quint32 mWidth;
  quint32 mHeight;
};

PELICAN_DECLARE_DATABLOB(StreamBlob)

#endif // STREAM_BLOB_H
