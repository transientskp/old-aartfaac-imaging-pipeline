#include "ServiceAdapter.h"
#include "ServiceBlob.h"
#include "../emulator/service/ServiceUdpPacket.h"

ServiceAdapter::ServiceAdapter(const ConfigNode &inConfig)
  : AbstractServiceAdapter(inConfig)
{
}

void ServiceAdapter::deserialise(QIODevice *inDevice)
{
  qDebug("DESERIALIZE! CHUNKSIZE: %lu", chunkSize());
  Q_UNUSED(inDevice);
}

