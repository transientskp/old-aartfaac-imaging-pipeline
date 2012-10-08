#include "ServiceEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

extern "C" void singles2halfp(void *target, void *source, int numel);

ServiceEmulator::ServiceEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode)
{
  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTimer.start();
}

ServiceEmulator::~ServiceEmulator()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void ServiceEmulator::getPacketData(char *&outData, unsigned long &outSize)
{
  outData = (char*) &mUdpPacket;
  outSize = sizeof(ServiceUdpPacket);
  memset(static_cast<void*>(&mUdpPacket), 0, sizeof(ServiceUdpPacket));
}

unsigned long ServiceEmulator::interval()
{
  return 0;
}

int ServiceEmulator::nPackets()
{
    return 0;
}

void ServiceEmulator::emulationFinished()
{
  QCoreApplication::exit(EXIT_SUCCESS);
}
