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
  quint32 mInterval;
  quint32 mSubbandSize;
  quint32 mPacketsPerSubband;

  quint16 mPort;
  QString mHost;

  QTime mTimer;

  char *mFullBandwidthData; ///< Full channel data of single time step
  char *mSubbandData;       ///< Data for a single packet (subset of mData)
  size_t mDataSize;         ///< mData size
  size_t mPacketSize;       ///< mPacket size
  size_t mBytesSend;        ///< Total bytes send

  casa::MeasurementSet *mMeasurementSet;
  casa::ROMSColumns *mMSColumns;
};

#endif // STREAM_EMULATOR_H
