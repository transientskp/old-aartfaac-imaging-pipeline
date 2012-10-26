#ifndef SERVICE_CHUNKER_H
#define SERVICE_CHUNKER_H

#include "../emulator/service/ServiceUdpPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>

using namespace pelican;

class ServiceChunker : public AbstractChunker
{
public:
  ServiceChunker(const ConfigNode &inConfig);

  virtual QIODevice *newDevice();
  virtual void next(QIODevice *inDevice);

private:
  qint64 mPacketSize; ///< Size of a udp packet in bytes
  qint64 mChunkSize; ///< Size of a chunk in bytes
  qint64 mBytesReceived; ///< Number of bytes received
  qint64 mAntennas; ///< Number of antennas
  WritableData *mChunk; ///< Chunk
};

PELICAN_DECLARE_CHUNKER(ServiceChunker)

#endif // SERVICE_CHUNKER_H
