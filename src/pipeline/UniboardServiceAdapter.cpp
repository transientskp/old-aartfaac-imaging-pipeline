#include "UniboardServiceAdapter.h"
#include "UniboardServiceBlob.h"

ServiceAdapter::ServiceAdapter(const ConfigNode &inConfig)
    : AbstractServiceAdapter(inConfig)
{
  qDebug("Initialized UniboardServiceAdapter");
}

void ServiceAdapter::deserialise(QIODevice *inDevice)
{
  qDebug("DESERIALISE!!!!!!!!!!!");
  Q_UNUSED(inDevice);
  qWarning("Chunksize: %lu", chunkSize());
}

