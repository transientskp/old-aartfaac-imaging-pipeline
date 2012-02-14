#ifndef DATA_CHUNKER_H
#define DATA_CHUNKER_H

#include <pelican/server/AbstractChunker.h>

using namespace pelican;

class DataChunker : public AbstractChunker
{
public:
    DataChunker(const ConfigNode& inConfig);

    virtual QIODevice* newDevice();
    virtual void next(QIODevice *inDevice);

private:
    qint64 mChunkSize;
    qint64 mBytesRead;
};

PELICAN_DECLARE_CHUNKER(DataChunker)

#endif // DATA_CHUNKER_H
