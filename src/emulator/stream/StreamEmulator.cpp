#include "StreamEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpSocket>

#include <casacore/ms/MeasurementSets.h>

StreamEmulator::StreamEmulator(const pelican::ConfigNode &configNode)
  : AbstractEmulator(),
    mTotalPackets(0),
    mRowIndex(0)
{
  mHost = configNode.getOption("connection", "host", "127.0.0.1");
  mPort = configNode.getOption("connection", "port", "2001").toShort();

  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  mTotalChannels = mMSColumns->spectralWindow().numChan()(0);
  mTotalAntennas = mMSColumns->antenna().nrow();

  if (mTotalAntennas != NUM_ANTENNAS)
    qFatal("Expecting %d antennas, got %d", NUM_ANTENNAS, mTotalAntennas);

  if (mMSColumns->spectralWindow().numChan().nrow() != 1)
    qFatal("Varying channels in single MS not supported");

  if (mTotalChannels > NUM_CHANNELS)
    qFatal("Too many channels in MS");

  mTimer.start();
}

StreamEmulator::~StreamEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void StreamEmulator::getPacketData(char *&data, unsigned long &size)
{
  data = (char *) &mUdpPacket;
  size = sizeof(StreamUdpPacket);

  // Reset the packet to 0
  memset(static_cast<void *>(&mUdpPacket), 0, sizeof(StreamUdpPacket));

  // Set the udp packet header
  mUdpPacket.mHeader.freq = mMSColumns->spectralWindow().chanFreq()(0).data()[0];
  mUdpPacket.mHeader.chan_width = mMSColumns->spectralWindow().chanWidth()(0).data()[0];
  mUdpPacket.mHeader.time = mMSColumns->time()(mRowIndex);
  mUdpPacket.mHeader.channels = mTotalChannels;

  // Set the udp packet data
  casa::Array<casa::Complex> data_array;

  for (int i = 0; i < NUM_BASELINES; i++)
  {
    if (mUdpPacket.mHeader.time != mMSColumns->time()(mRowIndex))
    {
      qCritical("[%s] MS different times in single packet", __PRETTY_FUNCTION__);
      break;
    }

    int a1 = mMSColumns->antenna1()(mRowIndex);
    int a2 = mMSColumns->antenna2()(mRowIndex);
    int baseline = a1*NUM_ANTENNAS+a2;

    data_array = mMSColumns->data()(mRowIndex);
    for (quint32 channel = 0; channel < mTotalChannels; channel++)
    {
      for (int x = 0; x < 2; x++)
      {
        for (int y = 0; y < 2; y++)
        {
          mUdpPacket.visibilities[baseline][channel][x][y] = 
            data_array(casa::IPosition(2, x*NUM_POLARIZATIONS+y, channel));
        }
      }
    }

    mRowIndex++;

    if (mRowIndex % (mTotalTableRows / 100) == 0)
       qDebug("Sent %3d%% of measurement set",
              int(ceil((mRowIndex / double(mTotalTableRows)) * 100)));
  }

  mTotalPackets++;
}

QIODevice* StreamEmulator::createDevice()
{
  QTcpSocket *socket = new QTcpSocket();
  socket->connectToHost(mHost, mPort);
  return socket;
}

unsigned long StreamEmulator::interval()
{
  return 0;
}

int StreamEmulator::nPackets()
{
  return mTotalTableRows / NUM_BASELINES;
}

void StreamEmulator::emulationFinished()
{
  float seconds = mTimer.elapsed() / 1000.0f;
  float mbytes = (sizeof(StreamUdpPacket) * mTotalPackets) / (1024.0f * 1024.0f);

  qDebug("Packet     : %ld bytes", sizeof(StreamUdpPacket));
  qDebug("Channels   : %d channels", mTotalChannels);
  qDebug("MBytes     : %0.2f sent", mbytes);
  qDebug("MB/sec     : %0.2f sent", mbytes / seconds);
  qDebug("Packets    : %d sent", mTotalPackets);

  QCoreApplication::quit();
}
