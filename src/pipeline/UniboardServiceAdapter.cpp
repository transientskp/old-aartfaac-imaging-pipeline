#include "UniboardServiceAdapter.h"
#include "UniboardServiceBlob.h"

UniboardServiceAdapter::UniboardServiceAdapter(const ConfigNode &inConfig)
    : AbstractServiceAdapter(inConfig)
{
  qDebug("Initialized UniboardServiceAdapter");
}

void UniboardServiceAdapter::deserialise(QIODevice *inDevice)
{
  qDebug("DESERIALISE!!!!!!!!!!!");
  Q_UNUSED(inDevice);
  qWarning("Chunksize: %lu", chunkSize());
}

