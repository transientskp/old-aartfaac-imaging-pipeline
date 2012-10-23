#ifndef UNIBOARD_SERVICE_ADAPTER_H
#define UNIBOARD_SERVICE_ADAPTER_H

#include <pelican/core/AbstractServiceAdapter.h>

using namespace pelican;

class UniboardServiceAdapter : public AbstractServiceAdapter
{
public:
  UniboardServiceAdapter(const ConfigNode &inConfig);

  void deserialise(QIODevice *inDevice);

private:

};

// Register the adapter
PELICAN_DECLARE_ADAPTER(UniboardServiceAdapter)

#endif // UNIBOARD_SERVICE_ADAPTER_H
