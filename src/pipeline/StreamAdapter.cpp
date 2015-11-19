#include "StreamAdapter.h"
#include "StreamBlob.h"
#include "Constants.h"
#include "../emulator/stream/StreamPacket.h"
#include "../utilities/Utils.h"

#include <complex>
#include <Eigen/Dense>

// Construct the example adapter.
StreamAdapter::StreamAdapter(const ConfigNode &config):
  AbstractStreamAdapter(config)
{
}

void StreamAdapter::deserialise(QIODevice *inDevice)
{
  StreamBlob *blob = static_cast<StreamBlob *>(dataBlob());

  size_t bytes_read = 0;
  while (size_t(inDevice->bytesAvailable()) < sizeof(ChunkHeader))
    inDevice->waitForReadyRead(-1);

  bytes_read += inDevice->read(reinterpret_cast<char*>(&(blob->mHeader)),
                               sizeof(ChunkHeader));
  blob->reset();

  while (size_t(inDevice->bytesAvailable()) < chunkSize()-sizeof(ChunkHeader))
    inDevice->waitForReadyRead(-1);

  bytes_read += inDevice->read(reinterpret_cast<char*>(blob->mRawData[0].data()),
                               blob->mRawData[0].size()*sizeof(std::complex<float>));
  bytes_read += inDevice->read(reinterpret_cast<char*>(blob->mRawData[1].data()),
                               blob->mRawData[1].size()*sizeof(std::complex<float>));

  Q_ASSERT(bytes_read == chunkSize());

  /*
  utils::matrix2stderr(blob->mRawData[0], "xx");
  utils::matrix2stderr(blob->mRawData[1], "yy");
  exit(1);
  */
}
