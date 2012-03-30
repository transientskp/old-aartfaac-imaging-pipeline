#ifndef UNIBOARD_DATABLOB_H
#define UNIBOARD_DATABLOB_H

#include <pelican/data/DataBlob.h>
#include <vector>
#include <complex>
#include <QtCore>

using namespace pelican;

class UniboardDataBlob : public DataBlob
{
public:
  UniboardDataBlob();

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

  void setFrequency(const double inFrequency)  { mFrequency = inFrequency; }
  quint32 getFrequency() const                 { return mFrequency; }
  double getMJDTime() const                    { return mMJDTime; }
  QDateTime getDateTime() const                { return mDateTime; }
  quint32 getWidth() const                     { return mWidth; }
  quint32 getHeight() const                    { return mHeight; }
  std::vector<float>& getSkyMap()              { return mSkyMap; }
  std::vector<float>& getVisMap()              { return mVisMap; }
  const std::vector<float>& getSkyMap() const  { return mSkyMap; }
  const std::vector<float>& getVisMap() const  { return mVisMap; }
  void createImage(const std::vector<unsigned char> &inData, const QString &inType);

private:
  double mMJDTime;
  double mFrequency;
  QDateTime mDateTime;
  quint32 mChannelId;
  quint32 mWidth;
  quint32 mHeight;

  std::vector<float> mXXReal; ///< xx polarized real part of the complex numbers
  std::vector<float> mXXImag; ///< xx polarized imaginary part of the complex numbers
  std::vector<float> mSkyMap; ///< raw skymap data from imager (needs normalization)
  std::vector<float> mVisMap; ///< raw vismap data from imager (needs normalization)
};

PELICAN_DECLARE_DATABLOB(UniboardDataBlob)

#endif // UNIBOARDDATABLOB_H
