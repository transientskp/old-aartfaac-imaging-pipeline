#ifndef UNIBOARD_CHUNKER_H
#define UNIBOARD_CHUNKER_H

#include "../emulator/UdpPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>

using namespace pelican;

class UniboardChunker : public AbstractChunker
{
public:
    UniboardChunker(const ConfigNode& inConfig);

    virtual QIODevice* newDevice();
    virtual void next(QIODevice *inDevice);

private:

    class Chunk {
    public:
      Chunk(UniboardChunker *inChunker);

      bool isTimeUp();
      bool isFilled();
      void addData(const void *inData, const int inLength);
      quint32 fill();

    private:
      WritableData mData;
      QTime mTimer;
      quint64 mBytesRead;
      UniboardChunker *mChunker;
      char *mPtr;
    };

    quint64 mChunkSize; ///< Size of a chunk in bytes
    qint64 mPacketSize; ///< Size of a udp packet
    int mTimeout; ///< Max time a chunk will wait for data in milliseconds
    static UdpPacket sEmptyPacket; ///< Default empty packet

    QHash<QString, Chunk*> mDataBuffers;
    QString hash(const double inTime, const quint32 inChannelId);
};

PELICAN_DECLARE_CHUNKER(UniboardChunker)

#endif // UNIBOARD_CHUNKER_H
