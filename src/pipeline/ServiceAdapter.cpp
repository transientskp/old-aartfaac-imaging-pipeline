#include "ServiceAdapter.h"
#include "ServiceBlob.h"
#include "../emulator/service/ServiceUdpPacket.h"

ServiceAdapter::ServiceAdapter(const ConfigNode &inConfig)
  : AbstractServiceAdapter(inConfig)
{
  mPacketSize = sizeof(ServiceUdpPacket);
}

void ServiceAdapter::deserialise(QIODevice *inDevice)
{
  qWarning("Received service data");
  quint32 num_packets = chunkSize() / mPacketSize;

  ServiceUdpPacket packet;

  for (quint32 i = 0; i < num_packets; i++)
  {
    while (inDevice->bytesAvailable() < mPacketSize)
      inDevice->waitForReadyRead(100);

    inDevice->read(reinterpret_cast<char *>(&packet), mPacketSize);
  }
}

