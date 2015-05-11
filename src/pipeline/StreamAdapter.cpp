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
  mTimer.start();
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

  Eigen::VectorXcf v(NUM_ANTENNAS);
  char *ptr = reinterpret_cast<char*>(v.data());
  for (int c = blob->mHeader.start_chan; c <= blob->mHeader.end_chan; c++)
  {
    for (int p = 0; p < NUM_USED_POLARIZATIONS; p++)
    {
      for (int i = 0; i < NUM_ANTENNAS; i++)
      {
        bytes_read += inDevice->read(ptr, sizeof(std::complex<float>)*(i+1));
        blob->addVis(c, p, i, v);
      }
    }
  }

//  utils::matrix2stderr(blob->mCleanData[0], "xx");
//  utils::matrix2stderr(blob->mCleanData[1], "yy");
//  exit(1);

  blob->computeStats();
  Q_ASSERT(bytes_read == chunkSize());
  float bps = bytes_read * 8 / (mTimer.elapsed() / 1000.0f);
  mTimer.restart();
  qDebug("Throughput: %0.2f Mb/s", bps/(1024*1024));
}
