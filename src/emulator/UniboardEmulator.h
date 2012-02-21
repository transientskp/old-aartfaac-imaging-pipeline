#ifndef UNIBOARD_EMULATOR_H
#define UNIBOARD_EMULATOR_H

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <casacore/ms/MeasurementSets.h>
#include <QtCore/QByteArray>

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

  quint64 mTotalSamples;
  quint64 mTotalPackets;
  quint64 mTotalTableRows;
  quint64 mSamples;
  quint64 mRowIndex;

  QByteArray mUdpPacket;

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // UNIBOARD_EMULATOR_H
