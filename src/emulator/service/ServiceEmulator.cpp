#include "ServiceEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

ServiceEmulator::ServiceEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode),
    mCurrentRow(0),
    mTotalPackets(0)
{
  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalRows = mMSColumns->antenna().nrow();
  mMaxRowsPerPacket = MAX_ANTENNAS;
  mTimer.start();
}

ServiceEmulator::~ServiceEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void ServiceEmulator::getPacketData(char *&outData, unsigned long &outSize)
{
  outData = (char *) (&mUdpPacket);
  outSize = sizeof(mUdpPacket);
  memset(static_cast<void *>(&mUdpPacket), 0, outSize);

  mUdpPacket.mHeader.antennae = mTotalRows;

  quint32 i;
  for (i = 0; i < mMaxRowsPerPacket && mCurrentRow < mTotalRows; i++)
  {
    mUdpPacket.mAntennas[i].id = mCurrentRow;
    casa::String name = mMSColumns->antenna().name()(mCurrentRow);
    strcpy(mUdpPacket.mAntennas[i].name, name.c_str());

    mCurrentRow++;

    if (mCurrentRow % (mTotalRows / 100) == 0)
      qDebug("Sent %3d%% of measurement set",
             int(floor((mCurrentRow / double(mTotalRows)) * 100.0)));
  }

  mUdpPacket.mHeader.rows = i;
  mTotalPackets++;
}

unsigned long ServiceEmulator::interval()
{
  return 0;
}

int ServiceEmulator::nPackets()
{
  return int(ceil(double(mTotalRows) / mMaxRowsPerPacket));
}

void ServiceEmulator::emulationFinished()
{
  float seconds = mTimer.elapsed() / 1000.0f;
  float mbytes = (sizeof(ServiceUdpPacket) * mTotalPackets) / (1024.0f * 1024.0f);

  qDebug("Header : %ld bytes", sizeof(ServiceUdpPacket::Header));
  qDebug("Antenna: %ld bytes", sizeof(ServiceUdpPacket::Antenna));
  qDebug("Packet : %ld bytes", sizeof(ServiceUdpPacket));
  qDebug("MBytes : %0.2f sent", mbytes);
  qDebug("MB/sec : %0.2f sent", mbytes / seconds);
  qDebug("Sent   : %lld samples", mCurrentRow);
  qDebug("Service Emulator finished");
}
