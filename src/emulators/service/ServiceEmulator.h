#ifndef UNIBOARD_EMULATOR_H
#define UNIBOARD_EMULATOR_H

#include "ServiceUdpPacket.h"

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <casacore/ms/MeasurementSets.h>
#include <QtCore/QByteArray>
#include <QTime>

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
};

#endif // UNIBOARD_EMULATOR_H
