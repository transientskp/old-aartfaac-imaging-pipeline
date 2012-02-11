#ifndef UNIBOARD_EMULATOR_H
#define UNIBOARD_EMULATOR_H

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Slicer.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <QtCore/QByteArray>

class UniboardEmulator : public pelican::AbstractUdpEmulator
{
public:
  UniboardEmulator(const pelican::ConfigNode &configNode);

  ~UniboardEmulator() {}

  void getPacketData(char*& ptr, unsigned long& size);

private:
  unsigned long long _totalSamples;
  unsigned long long _totalPackets;
  unsigned long long _tableRows;
  unsigned long _samples;


  QByteArray _packet;

  casa::Table _table;
  casa::ROScalarColumn<casa::Double> _timeColumn;
  casa::ROArrayColumn<casa::Complex> _dataColumn;
  casa::ROArrayColumn<casa::Double> _posColumn;
};

#endif // UNIBOARD_EMULATOR_H
