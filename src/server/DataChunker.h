#ifndef DATA_CHUNKER_H
#define DATA_CHUNKER_H

#include <pelican/server/AbstractChunker.h>

using namespace pelican;

class DataChunker : public AbstractChunker
{
public:
    DataChunker(const ConfigNode& config);

    virtual QIODevice* newDevice();
    virtual void next(QIODevice *);

private:
    qint64 _chunkSize;
    qint64 _bytesRead;
};

PELICAN_DECLARE_CHUNKER(DataChunker)

#endif // DATA_CHUNKER_H
