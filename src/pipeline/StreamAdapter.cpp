#include "StreamAdapter.h"
#include "StreamBlob.h"
#include "Constants.h"
#include "../emulator/stream/StreamPacket.h"

#include <complex>

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
    inDevice->waitForReadyRead(100);

  std::complex<float> v[NUM_POLARIZATIONS];
  char *ptr = reinterpret_cast<char*>(v);
  for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
  {
    for (int a1 = 0; a1 < (a2 + 1); a1++)
    {
      for (int c = blob->mHeader.start_chan; c <= blob->mHeader.end_chan; c++)
      {
        bytes_read += inDevice->read(ptr, sizeof(std::complex<float>)*NUM_POLARIZATIONS);
        blob->addVis(a1, a2, v);
      }
    }
  }

  blob->computeStats();
  Q_ASSERT(bytes_read == chunkSize());
}
