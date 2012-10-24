#ifndef UNIBOARD_SERVICE_ADAPTER_H
#define UNIBOARD_SERVICE_ADAPTER_H

#include <pelican/core/AbstractServiceAdapter.h>

using namespace pelican;

class ServiceAdapter : public AbstractServiceAdapter
{
public:
  ServiceAdapter(const ConfigNode &inConfig);

  void deserialise(QIODevice *inDevice);

private:

};

// Register the adapter
PELICAN_DECLARE_ADAPTER(ServiceAdapter)

#endif // UNIBOARD_SERVICE_ADAPTER_H
