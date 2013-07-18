#include "StreamEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpSocket>

#include <casacore/ms/MeasurementSets.h>

StreamEmulator::StreamEmulator(const pelican::ConfigNode &configNode):
  AbstractEmulator(),
  mTotalPackets(0),
  mRowIndex(0),
  mBytesSend(0)
{
  Q_ASSERT(sizeof(casa::Complex) == sizeof(std::complex<float>));
  Q_ASSERT(sizeof(StreamHeader) == 512);

  mHost = configNode.getOption("connection", "host");
  mPort = configNode.getOption("connection", "port").toShort();
  mInterval = configNode.getOption("emulator", "packetInterval").toInt();
  mSubbandSize = configNode.getOption("emulator", "subbandSize").toInt();
  QString table_name = configNode.getOption("measurementset", "name");

  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  mTotalChannels = mMSColumns->spectralWindow().numChan()(0);
  mTotalAntennas = mMSColumns->antenna().nrow();

  if (mSubbandSize > mTotalChannels)
    qFatal("Size of a subband cannot exceed number of channels in MS");

  if (mTotalTableRows % NUM_BASELINES != 0)
    qFatal("Expecting number of tablerows %d to be a multiple of %d", mTotalTableRows, NUM_BASELINES);

  if (mTotalAntennas != NUM_ANTENNAS)
    qFatal("Expecting %d antennas, got %d", NUM_ANTENNAS, mTotalAntennas);

  if (mMSColumns->spectralWindow().numChan().nrow() != 1)
    qFatal("Varying channels in single MS not supported");

  double freq = mMSColumns->spectralWindow().chanFreq()(0).data()[0];
  double freq_width = mMSColumns->spectralWindow().chanWidth()(0).data()[0];

  qDebug("Host         : %s:%d", qPrintable(mHost), mPort);
  qDebug("Interval     : %d", mInterval);
  qDebug("Subband size : %d", mSubbandSize);
  qDebug("Table name   : %s", qPrintable(table_name));
  std::cout << std::endl;
  qDebug("Number of channels  : %u", mTotalChannels);
  qDebug("Reference frequency : %f", freq);
  qDebug("Channel width       : %f", freq_width);

  // Allocate the data as local buffers
  mDataSize = NUM_BASELINES*mTotalChannels*NUM_POLARIZATIONS*sizeof(std::complex<float>);
  mPacketSize = sizeof(StreamHeader) + NUM_BASELINES*mSubbandSize*NUM_POLARIZATIONS*sizeof(std::complex<float>);
  mData = new char[mDataSize];
  mPacket = new char[mPacketSize];
  memset(static_cast<void*>(mData), 0, mDataSize);
  memset(static_cast<void*>(mPacket), 0, mPacketSize);
  mPacketsPerSubband = mTotalChannels / mSubbandSize;
}

StreamEmulator::~StreamEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
  delete[] mData;
  delete[] mPacket;
}

void StreamEmulator::getPacketData(char *&data, unsigned long &size)
{
  data = mPacket;
  size_t full_baseline_size = mTotalChannels*NUM_POLARIZATIONS*sizeof(std::complex<float>);
  size_t subband_baseline_size = mSubbandSize*NUM_POLARIZATIONS*sizeof(std::complex<float>);

  // New subband and timeslot, load new data from ms and send header + first packet of this batch
  quint32 subband = mTotalPackets % mPacketsPerSubband;
  if (subband == 0)
  {
    size = mPacketSize;

    // Get header memory
    StreamHeader *header = reinterpret_cast<StreamHeader*>(mPacket);

    // Set the packet header
    header->magic = HEADER_MAGIC;
    header->start_time = header->end_time;
    header->end_time = mMSColumns->time()(mRowIndex);

    // Load all the data for this timeslot per baseline
    for (quint32 i = 0; i < NUM_BASELINES; i++)
    {
      // Load the data from casa measurement set
      casa::Array<casa::Complex> data_array(
            casa::IPosition(2, NUM_POLARIZATIONS, mTotalChannels),
            reinterpret_cast<casa::Complex*>(mData + i*full_baseline_size),
            casa::SHARE);

      mMSColumns->data().get(mRowIndex, data_array);
      mRowIndex++;
    }

    // Construct the first packet of this batch
    for (quint32 i = 0; i < NUM_BASELINES; i++)
    {
      memcpy(
        static_cast<void*>(mPacket + sizeof(StreamHeader) + i*subband_baseline_size),
        static_cast<void*>(mData + i*full_baseline_size),
        subband_baseline_size
      );
    }
  }
  // Send remaining packets belonging to this timeslot
  else
  {
    size = mPacketSize - sizeof(StreamHeader);

    // Construct the remaining packet of this batch
    for (quint32 i = 0; i < NUM_BASELINES; i++)
    {
      memcpy(
        static_cast<void*>(mPacket + i*subband_baseline_size),
        static_cast<void*>(mData + i*full_baseline_size + subband*subband_baseline_size),
        subband_baseline_size
      );
    }
  }

  mBytesSend += size;
  std::cout << nPackets() - mTotalPackets << " " << std::flush;
  mTotalPackets++;
}

QIODevice* StreamEmulator::createDevice()
{
  QTcpSocket *socket = new QTcpSocket();
  socket->abort();
  socket->connectToHost(mHost, mPort);
  if (!socket->waitForConnected(-1))
    throw QString("Error: ") + socket->errorString();

  std::cout << std::endl << "Sending..." << std::flush;
  mTimer.start();
  return socket;
}

unsigned long StreamEmulator::interval()
{
  return mInterval;
}

int StreamEmulator::nPackets()
{
  return (mTotalTableRows / NUM_BASELINES) * mPacketsPerSubband;
}

void StreamEmulator::emulationFinished()
{
  float seconds = mTimer.elapsed() / 1000.0f;

  std::cout << "[done]" << std::endl << std::endl;

  qDebug("Packet     : %ld bytes", mPacketSize);
  qDebug("Speed      : %0.2f MiB/s", (mBytesSend / (1024.0f*1024.0f)) / seconds);
  qDebug("Total sent : %ld bytes, %d packets", mBytesSend, mTotalPackets);

  QCoreApplication::quit();
}
