#ifndef SERVICE_EMULATOR_H
#define SERVICE_EMULATOR_H

#include "ServiceUdpPacket.h"

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <QtCore/QByteArray>
#include <QTime>

namespace casa
{
  class MeasurementSet;
  class ROMSColumns;
}

class ServiceEmulator : public pelican::AbstractUdpEmulator
{
public:
  ServiceEmulator(const pelican::ConfigNode &inConfigNode);
  ~ServiceEmulator();

  void getPacketData(char *&outData, unsigned long &outSize);
  unsigned long interval();
  int nPackets();

private:
  void emulationFinished();

  QTime mTimer;
  ServiceUdpPacket mUdpPacket;

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;

  quint64 mTotalRows;
  quint64 mMaxRowsPerPacket;
  quint64 mCurrentRow;
  quint64 mTotalPackets;
};

#endif // SERVICE_EMULATOR_H
