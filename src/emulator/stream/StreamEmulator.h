#ifndef STREAM_EMULATOR_H
#define STREAM_EMULATOR_H

#include "StreamPacket.h"

#include <pelican/emulator/AbstractEmulator.h>
#include <pelican/utility/ConfigNode.h>

#include <QTime>

namespace casa
{
  class MeasurementSet;
  class ROMSColumns;
}

class StreamEmulator : public pelican::AbstractEmulator
{
public:
  StreamEmulator(const pelican::ConfigNode &configNode);
  ~StreamEmulator();

  void getPacketData(char *&data, unsigned long &size);
  unsigned long interval();
  int nPackets();

private:
  void emulationFinished();
  QIODevice* createDevice();

  quint32 mTotalPackets;
  quint32 mTotalTableRows;
  quint32 mTotalChannels;
  quint32 mTotalAntennas;
  quint32 mRowIndex;

  quint16 mPort;
  QString mHost;

  QTime mTimer;

  char *mData;
  size_t mDataSize;

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // STREAM_EMULATOR_H
