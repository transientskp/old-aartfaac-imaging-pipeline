#ifndef UNIBOARD_ADAPTER_H
#define UNIBOARD_ADAPTER_H

#include <pelican/core/AbstractStreamAdapter.h>

using namespace pelican;

class UniboardAdapter : public AbstractStreamAdapter
{
public:
  /// Constructs the uniboard adapter.
  UniboardAdapter(const ConfigNode& inConfig);

  /// Method to deserialise chunks of memory provided by the I/O device.
  void deserialise(QIODevice *inDevice);

private:
  quint32 mMaxPacketSamples;
  quint32 mAntennae;
  qint64 mPacketSize;
};

// Register the adapter.
PELICAN_DECLARE_ADAPTER(UniboardAdapter)

#endif // UNIBOARD_ADAPTER_H