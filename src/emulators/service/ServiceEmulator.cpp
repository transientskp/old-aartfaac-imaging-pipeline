#include "ServiceEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

ServiceEmulator::ServiceEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode),
  mCurrentRow(0)
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
  outData = (char*) (&mUdpPacket);
  outSize = sizeof(mUdpPacket);
  memset(static_cast<void*>(&mUdpPacket), 0, outSize);

  casa::Array<casa::Double> double_array;
  casa::Array<casa::Double>::iterator iter;
  int j = 0;
  for (quint32 i = 0; i < mMaxRowsPerPacket; i++)
  {
    double_array = mMSColumns->antenna().offset()(mCurrentRow);
    j = 0;
    for (iter = double_array.begin(); iter != double_array.end(); ++iter)
      mUdpPacket.mAntennas[i].offset[j++] = *iter;

    double_array = mMSColumns->antenna().position()(mCurrentRow);
    j = 0;
    for (iter = double_array.begin(); iter != double_array.end(); ++iter)
      mUdpPacket.mAntennas[i].pos[j++] = *iter;

    mCurrentRow++;
  }
}

unsigned long ServiceEmulator::interval()
{
  return 0;
}

int ServiceEmulator::nPackets()
{
  return (mTotalRows / mMaxRowsPerPacket) + (mTotalRows % mMaxRowsPerPacket);
}

void ServiceEmulator::emulationFinished()
{
  QCoreApplication::exit(EXIT_SUCCESS);
}
