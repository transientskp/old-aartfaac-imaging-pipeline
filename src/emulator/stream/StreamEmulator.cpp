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

  mPacket = new StreamPacket();
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

  std::cout << "Sending..." << std::flush;
  mTimer.start();
}

StreamEmulator::~StreamEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void StreamEmulator::getPacketData(char *&data, unsigned long &size)
{
  data = (char *) mPacket;
  size = sizeof(StreamPacket);

  // Reset the packet header to 0
  memset(static_cast<void *>(&mPacket->mHeader), 0, sizeof(StreamPacket::Header));

  // Set the packet header
  mPacket->mHeader.freq = mMSColumns->spectralWindow().chanFreq()(0).data()[0];
  mPacket->mHeader.chan_width = mMSColumns->spectralWindow().chanWidth()(0).data()[0];
  mPacket->mHeader.time = mMSColumns->time()(mRowIndex);
  mPacket->mHeader.channels = mTotalChannels;

  // Set the packet data
  casa::Array<casa::Complex> data_array;

  for (int i = 0; i < NUM_BASELINES; i++)
  {
    if (mPacket->mHeader.time != mMSColumns->time()(mRowIndex))
    {
      qCritical("[%s] MS different times in single packet", __PRETTY_FUNCTION__);
      break;
    }

    int a1 = mMSColumns->antenna1()(mRowIndex);
    int a2 = mMSColumns->antenna2()(mRowIndex);
    int baseline = a2*(a2+1)/2 + a1;

    data_array = mMSColumns->data()(mRowIndex);
    for (quint32 channel = 0; channel < mTotalChannels; channel++)
      for (int pol = 0; pol < NUM_POLARIZATIONS; pol++)
        mPacket->visibilities[baseline][channel][pol] =
          data_array(casa::IPosition(2, pol, channel));

    mRowIndex++;
    if (mRowIndex % (mTotalTableRows / 10) == 0)
    {
      static int countdown = 10;
      std::cout << countdown-- << " " << std::flush;
    }
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
  float mbytes = (sizeof(StreamPacket) * mTotalPackets) / (1024.0f * 1024.0f);

  std::cout << "[done]" << std::endl << std::endl;

  qDebug("Packet     : %ld bytes", sizeof(StreamPacket));
  qDebug("Channels   : %d channels", mTotalChannels);
  qDebug("MBytes     : %0.2f sent", mbytes);
  qDebug("MB/sec     : %0.2f sent", mbytes / seconds);
  qDebug("Packets    : %d sent", mTotalPackets);

  delete mPacket;
  QCoreApplication::quit();
}
