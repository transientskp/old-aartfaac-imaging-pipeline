#ifndef STREAM_EMULATOR_H
#define STREAM_EMULATOR_H

#include "StreamUdpPacket.h"

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <casacore/ms/MeasurementSets.h>
#include <QtCore/QByteArray>
#include <QTime>

class StreamEmulator : public pelican::AbstractUdpEmulator
{
public:
  StreamEmulator(const pelican::ConfigNode &inConfigNode);
  ~StreamEmulator();

  void getPacketData(char *&outData, unsigned long &outSize);
  unsigned long interval();
  int nPackets();

private:
  void emulationFinished();

  quint64 mTotalCorrelations;
  quint64 mTotalPackets;
  quint64 mTotalTableRows;
  quint64 mTotalChannelsAndTableRows;
  quint64 mTotalChannels;
  quint64 mCurChannelId;
  quint64 mMaxSamples;
  quint64 mRowIndex;
  quint64 mTotalRowIndex;

  QTime mTimer;
  StreamUdpPacket mUdpPacket;

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // STREAM_EMULATOR_H
