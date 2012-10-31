#ifndef META_DATA_H
#define META_DATA_H

#include <casacore/ms/MeasurementSets.h>
#include <QtCore>

class MetaData
{
public:
  MetaData();
  ~MetaData();

  void start();

private:
  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // META_DATA_H
