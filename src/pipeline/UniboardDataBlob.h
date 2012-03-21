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

  void setChannelId(const quint32 inChannelId) { mChannelId = inChannelId; }

  quint32 getChannelId() const                 { return mChannelId; }

  double getMJDTime() const                    { return mMJDTime; }

  QDateTime getDateTime() const                { return mDateTime; }

  const std::vector<float>* getXXReal() const;

  const std::vector<float>* getXXImag() const;

  void createImage(const std::vector<unsigned char> &inData, const QString &inType);

private:
  double mMJDTime;
  QDateTime mDateTime;
  quint32 mChannelId;

  std::vector<float> mXXReal;
  std::vector<float> mXXImag;
};

PELICAN_DECLARE_DATABLOB(UniboardDataBlob)

#endif // UNIBOARDDATABLOB_H
