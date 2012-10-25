#include "ServiceAdapter.h"
#include "ServiceBlob.h"

ServiceAdapter::ServiceAdapter(const ConfigNode &inConfig)
  : AbstractServiceAdapter(inConfig)
{
}

void ServiceAdapter::deserialise(QIODevice *inDevice)
{
  Q_UNUSED(inDevice);
}

