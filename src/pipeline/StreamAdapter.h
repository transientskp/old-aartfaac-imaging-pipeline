#ifndef STREAM_ADAPTER_H
#define STREAM_ADAPTER_H

#include <pelican/core/AbstractStreamAdapter.h>
#include <QtCore>
#include <Eigen/Dense>

using namespace pelican;

class StreamAdapter : public AbstractStreamAdapter
{
public:
  /// Constructs the uniboard adapter.
  StreamAdapter(const ConfigNode &inConfig);

  /// Method to deserialise chunks of memory provided by the I/O device.
  void deserialise(QIODevice *inDevice);

private:
  QTime mTimer;
};

// Register the adapter.
PELICAN_DECLARE_ADAPTER(StreamAdapter)

#endif // STREAM_ADAPTER_H
