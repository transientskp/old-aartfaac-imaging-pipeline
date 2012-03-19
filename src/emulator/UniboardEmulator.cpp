#include "UniboardEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

extern "C" void singles2halfp(void *target, void *source, int numel);

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode)
{
  mTotalPackets = 0;
  mTotalSamples = 0;
  mRowIndex     = 0;

  mMaxSamples = inConfigNode.getOption("packet", "samples").toULong();

  qDebug("Header: %ld bytes", sizeof(UdpPacket::Header));
  qDebug("Sample: %ld bytes", sizeof(UdpPacket::Correlation));
  qDebug("Packet: %ld bytes", sizeof(UdpPacket));

  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  qDebug("Sorting table...");
//  table = table.sort("TIME");
  qDebug("Sending data...");
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  mTimer.start();
}

UniboardEmulator::~UniboardEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void UniboardEmulator::getPacketData(char *&outData, unsigned long &outSize)
{
  outData = (char*) &mUdpPacket;
  outSize = sizeof(UdpPacket);
  memset(static_cast<void*>(&mUdpPacket), 0, sizeof(UdpPacket));

  Q_ASSERT(mMSColumns->spectralWindow().numChan().nrow() == 1);

  qint32 channel_id = mMSColumns->spectralWindow().numChan()(0);
  double cur_time = mMSColumns->time()(mRowIndex);

  // Set the udp packet header
  mUdpPacket.mHeader.time    = cur_time;
  mUdpPacket.mHeader.channel = channel_id;
  mUdpPacket.mHeader.samples = 0;

  // Set the udp packet data
  casa::Array<casa::Complex> data_array;
  casa::Array<casa::Complex>::iterator iter;
  for (quint32 i = 0; i < mMaxSamples && mRowIndex < mTotalTableRows; i++)
  {
    UdpPacket::Correlation &sample = mUdpPacket.mSamples[i];
    double time = mMSColumns->time()(mRowIndex);

    // time changed within packet, break and send
    if (time != cur_time)
      break;

    sample.a1   = mMSColumns->antenna1()(mRowIndex);
    sample.a2   = mMSColumns->antenna2()(mRowIndex);
    data_array  = mMSColumns->data()(mRowIndex);

    // Fill sample with complex data polarizations
    int j = 0;
    for (iter = data_array.begin(); iter != data_array.end(); iter++)
    {
      singles2halfp(static_cast<quint16*>(&sample.polarizations[j++]),
                    static_cast<void*>(&(*iter).real()), 1);

      singles2halfp(static_cast<quint16*>(&sample.polarizations[j++]),
                    static_cast<void*>(&(*iter).imag()), 1);
    }

    mUdpPacket.mHeader.samples++;
    mRowIndex++;

    if (mRowIndex % (mTotalTableRows / 100) == 0)
      qDebug("Sent %3d%% of measurement set",
             int(ceil((mRowIndex / double(mTotalTableRows))*100)));
  }

  // Increase counters
  mTotalPackets++;
  mTotalSamples += mUdpPacket.mHeader.samples;
}

unsigned long UniboardEmulator::interval()
{
  return 0;
}

int UniboardEmulator::nPackets()
{
  return (mTotalTableRows / mMaxSamples) + (mTotalTableRows % mMaxSamples);
}

void UniboardEmulator::emulationFinished()
{
  float seconds = mTimer.elapsed() / 1000.0f;
  float mbytes = (sizeof(UdpPacket) * mTotalPackets) / (1024.0f * 1024.0f);
  qDebug("MBytes: %0.2f sent", mbytes);
  qDebug("MB/sec: %0.2f sent", mbytes/seconds);
  qDebug("Sent  : %lld samples", mTotalSamples);
  QCoreApplication::exit(EXIT_SUCCESS);
}
