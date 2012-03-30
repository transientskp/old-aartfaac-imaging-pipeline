#ifndef UNIBOARD_EMULATOR_H
#define UNIBOARD_EMULATOR_H

#include "UdpPacket.h"

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <casacore/ms/MeasurementSets.h>
#include <QtCore/QByteArray>
#include <QTime>

class UniboardEmulator : public pelican::AbstractUdpEmulator
{
public:
  UniboardEmulator(const pelican::ConfigNode &inConfigNode);
  ~UniboardEmulator();

  void getPacketData(char *&outData, unsigned long &outSize);
  unsigned long interval();
  int nPackets();

private:
  void emulationFinished();

  quint64 mTotalCorrelations;
  quint64 mTotalPackets;
  quint64 mTotalTableRows;
  quint64 mMaxSamples;
  quint64 mRowIndex;

  QTime mTimer;
  UdpPacket mUdpPacket;

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // UNIBOARD_EMULATOR_H
