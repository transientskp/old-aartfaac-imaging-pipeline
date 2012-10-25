#ifndef STREAM_CHUNKER_H
#define STREAM_CHUNKER_H

#include "../emulator/stream/StreamUdpPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>

using namespace pelican;

class StreamChunker : public AbstractChunker
{
public:
  StreamChunker(const ConfigNode &inConfig);

  virtual QIODevice *newDevice();
  virtual void next(QIODevice *inDevice);

private:

  class Chunk
  {
  public:
    Chunk(StreamChunker *inChunker);

    bool isTimeUp();
    bool isFilled();
    void addData(const void *inData, const int inLength);
    quint32 fill();

  private:
    WritableData mData;
    QTime mTimer;
    quint64 mBytesRead;
    StreamChunker *mChunker;
    char *mPtr;
  };

  quint64 mChunkSize; ///< Size of a chunk in bytes
  qint64 mPacketSize; ///< Size of a udp packet in bytes
  int mTimeout; ///< Max time a chunk will wait for data in milliseconds
  static StreamUdpPacket sEmptyPacket; ///< Default empty packet

  QHash<quint64, Chunk *> mDataBuffers;
  quint64 hash(const double inTime, const double inFrequency);
};

PELICAN_DECLARE_CHUNKER(StreamChunker)

#endif // STREAM_CHUNKER_H
