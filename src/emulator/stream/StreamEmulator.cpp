#include "StreamEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpSocket>

#include <casacore/ms/MeasurementSets.h>

StreamEmulator::StreamEmulator(const pelican::ConfigNode &configNode):
  AbstractEmulator(),
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

  if (mTotalChannels != NUM_CHANNELS)
    qWarning("Unexpected number of channels");

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
  memset(static_cast<void*>(&mPacket->mHeader), 0, sizeof(StreamPacket::Header));

  // Set the packet header
  mPacket->mHeader.channels = mTotalChannels;
  mPacket->mHeader.freq = mMSColumns->spectralWindow().chanFreq()(0).data()[0];
  mPacket->mHeader.chan_width = mMSColumns->spectralWindow().chanWidth()(0).data()[0];
  mPacket->mHeader.time = mMSColumns->time()(mRowIndex);
  mPacket->mHeader.a1 = mMSColumns->antenna1()(mRowIndex);
  mPacket->mHeader.a2 = mMSColumns->antenna2()(mRowIndex);

  // Load the data from casa measurement set
  casa::Array<casa::Complex> data_array(
        casa::IPosition(2, NUM_POLARIZATIONS, mTotalChannels),
        reinterpret_cast<casa::Complex*>(mPacket->visibilities),
        casa::SHARE);

  mMSColumns->data().get(mRowIndex, data_array);

  // Increase counters
  mRowIndex++;
  if (mRowIndex % (mTotalTableRows / 10) == 0)
  {
    static int countdown = 10;
    std::cout << countdown-- << " " << std::flush;
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
  return mTotalTableRows;
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
