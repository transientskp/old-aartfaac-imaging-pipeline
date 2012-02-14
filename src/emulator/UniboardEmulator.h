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
  UniboardEmulator(const pelican::ConfigNode &inConfigNode);

  ~UniboardEmulator() {}

  void getPacketData(char *&outData, unsigned long &outSize);

private:
  quint64 mTotalSamples;
  quint64 mTotalPackets;
  quint64 mTotalTableRows;
  quint64 mSamples;

  QByteArray mUdpPacket;

  casa::Table mTable;
  casa::ROScalarColumn<casa::Double> mTimeColumn;
  casa::ROArrayColumn<casa::Complex> mDataColumn;
};

#endif // UNIBOARD_EMULATOR_H
