#ifndef UNIBOARD_CHUNKER_H
#define UNIBOARD_CHUNKER_H

#include "../emulators/stream/StreamUdpPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>

using namespace pelican;

class StreamUniboardChunker : public AbstractChunker
{
public:
    StreamUniboardChunker(const ConfigNode& inConfig);

    virtual QIODevice* newDevice();
    virtual void next(QIODevice *inDevice);

private:

    class Chunk {
    public:
      Chunk(StreamUniboardChunker *inChunker);

      bool isTimeUp();
      bool isFilled();
      void addData(const void *inData, const int inLength);
      quint32 fill();

    private:
      WritableData mData;
      QTime mTimer;
      quint64 mBytesRead;
      StreamUniboardChunker *mChunker;
      char *mPtr;
    };

    quint64 mChunkSize; ///< Size of a chunk in bytes
    qint64 mPacketSize; ///< Size of a udp packet
    int mTimeout; ///< Max time a chunk will wait for data in milliseconds
    static StreamUdpPacket sEmptyPacket; ///< Default empty packet

    QHash<quint64, Chunk*> mDataBuffers;
    quint64 hash(const double inTime, const double inFrequency);
};

PELICAN_DECLARE_CHUNKER(StreamUniboardChunker)

#endif // UNIBOARD_CHUNKER_H
