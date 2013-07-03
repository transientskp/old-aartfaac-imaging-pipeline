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
  Q_ASSERT(sizeof(casa::Complex) == sizeof(std::complex<float>));
  Q_ASSERT(sizeof(StreamHeader) == 512);

  mHost = configNode.getOption("connection", "host", "127.0.0.1");
  mPort = configNode.getOption("connection", "port", "2001").toShort();
  QString table_name = configNode.getOption("measurementset", "name");

  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  mTotalChannels = mMSColumns->spectralWindow().numChan()(0);
  mTotalAntennas = mMSColumns->antenna().nrow();

  if (mTotalTableRows % NUM_BASELINES != 0)
    qFatal("Expecting number of tablerows %d to be a multiple of %d", mTotalTableRows, NUM_BASELINES);

  if (mTotalAntennas != NUM_ANTENNAS)
    qFatal("Expecting %d antennas, got %d", NUM_ANTENNAS, mTotalAntennas);

  if (mMSColumns->spectralWindow().numChan().nrow() != 1)
    qFatal("Varying channels in single MS not supported");

  double freq = mMSColumns->spectralWindow().chanFreq()(0).data()[0];
  double freq_width = mMSColumns->spectralWindow().chanWidth()(0).data()[0];

  qDebug("Number of channels  : %u", mTotalChannels);
  qDebug("Reference frequency : %f", freq);
  qDebug("Channel width       : %f", freq_width);

  // Allocate the data as local buffer
  mDataSize = sizeof(StreamHeader) + NUM_BASELINES*mTotalChannels*NUM_POLARIZATIONS*sizeof(std::complex<float>);
  mData = new char[mDataSize];
  memset(static_cast<void*>(mData), 0, mDataSize);
}

StreamEmulator::~StreamEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
  delete[] mData;
}

void StreamEmulator::getPacketData(char *&data, unsigned long &size)
{
  data = mData;
  size = mDataSize;

  // Get header memory
  StreamHeader *header = reinterpret_cast<StreamHeader*>(mData);

  // Set the packet header
  header->magic = HEADER_MAGIC;
  header->start_time = header->end_time;
  header->end_time = mMSColumns->time()(mRowIndex);

  // Load the data per baseline
  size_t baseline_size = mTotalChannels*NUM_POLARIZATIONS*sizeof(std::complex<float>);
  for (quint32 i = 0; i < NUM_BASELINES; i++)
  {
    // Load the data from casa measurement set
    casa::Array<casa::Complex> data_array(
          casa::IPosition(2, NUM_POLARIZATIONS, mTotalChannels),
          reinterpret_cast<casa::Complex*>(mData + sizeof(StreamHeader) + i*baseline_size),
          casa::SHARE);

    mMSColumns->data().get(mRowIndex, data_array);

    // Increase counters
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
  socket->abort();
  socket->connectToHost(mHost, mPort);

  if (!socket->waitForConnected(-1))
    qFatal("Error: Could not connect to %s:%d", qPrintable(mHost), mPort);

  std::cout << std::endl << "Sending..." << std::flush;
  mTimer.start();
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

  std::cout << "[done]" << std::endl << std::endl;

  size_t total_bytes = mTotalPackets * mDataSize;
  qDebug("Packet     : %ld bytes", mDataSize);
  qDebug("Speed      : %0.2f MiB/s", (total_bytes / (1024.0f*1024.0f)) / seconds);
  qDebug("Total sent : %ld bytes, %d packets", total_bytes, mTotalPackets);

  QCoreApplication::quit();
}
