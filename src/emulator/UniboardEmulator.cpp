#include "UniboardEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

extern "C" void singles2halfp(void *target, void *source, int numel);

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode)
{
  mTotalPackets = 0;
  mTotalCorrelations = 0;
  mRowIndex     = 0;
  mTotalRowIndex = 0;

  mMaxSamples = inConfigNode.getOption("packet", "samples").toULong();



  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  mTotalChannels = mMSColumns->spectralWindow().numChan()(0);
  mTotalChannelsAndTableRows = mTotalTableRows * mTotalChannels;
  mCurChannelId = 0;
  if (mMSColumns->spectralWindow().numChan().nrow() != 1)
    qFatal("Varying channels in single MS not supported");
  qDebug("Header     : %ld bytes", sizeof(UdpPacket::Header));
  qDebug("Correlation: %ld bytes", sizeof(UdpPacket::Correlation));
  qDebug("Packet     : %ld bytes", sizeof(UdpPacket));
  qDebug("Channels   : %lld channels", mTotalChannels);
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

  double freq = mMSColumns->spectralWindow().chanFreq()(0).data()[mCurChannelId];
  double cur_time = mMSColumns->time()(mRowIndex);

  // Set the udp packet header
  mUdpPacket.mHeader.time         = cur_time;
  mUdpPacket.mHeader.freq         = freq;
  mUdpPacket.mHeader.correlations = 0;

  // Set the udp packet data
  casa::Array<casa::Complex> data_array;
  casa::Array<casa::Complex>::iterator cIter;
  for (quint32 i = 0; i < mMaxSamples && mRowIndex < mTotalTableRows; i++)
  {
    UdpPacket::Correlation &correlation = mUdpPacket.mCorrelations[i];
    double time = mMSColumns->time()(mRowIndex);

    // time changed within packet, break and send
    if (time != cur_time)
      break;

    correlation.a1  = mMSColumns->antenna1()(mRowIndex);
    correlation.a2  = mMSColumns->antenna2()(mRowIndex);
    data_array = mMSColumns->data()(mRowIndex)[mCurChannelId];

    // Fill sample with complex data polarizations
    int j = 0;
    for (cIter = data_array.begin(); cIter != data_array.end(); cIter++)
    {
      singles2halfp(static_cast<quint16*>(&correlation.polarizations[j++]),
                    static_cast<void*>(&(*cIter).real()), 1);

      singles2halfp(static_cast<quint16*>(&correlation.polarizations[j++]),
                    static_cast<void*>(&(*cIter).imag()), 1);
    }

    mUdpPacket.mHeader.correlations++;
    mRowIndex++;
    mTotalRowIndex++;

    if (mTotalRowIndex % (mTotalChannelsAndTableRows / 100) == 0)
      qDebug("Sent %3d%% of measurement set",
             int(ceil((mTotalRowIndex / double(mTotalChannelsAndTableRows))*100)));
  }

  if (mRowIndex >= mTotalTableRows)
  {
    mRowIndex = 0;
    mCurChannelId++;
  }

  // Increase counters
  mTotalPackets++;
  mTotalCorrelations += mUdpPacket.mHeader.correlations;
}

unsigned long UniboardEmulator::interval()
{
  return 0;
}

int UniboardEmulator::nPackets()
{
  return (mTotalChannelsAndTableRows / mMaxSamples) + (mTotalChannelsAndTableRows % mMaxSamples);
}

void UniboardEmulator::emulationFinished()
{
  float seconds = mTimer.elapsed() / 1000.0f;
  float mbytes = (sizeof(UdpPacket) * mTotalPackets) / (1024.0f * 1024.0f);
  qDebug("MBytes:      %0.2f sent", mbytes);
  qDebug("MB/sec:      %0.2f sent", mbytes/seconds);
  qDebug("Sent  :      %lld samples", mTotalCorrelations);
  QCoreApplication::exit(EXIT_SUCCESS);
}
