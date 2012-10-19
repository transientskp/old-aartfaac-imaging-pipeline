#ifndef SERVICE_UNIBOARD_CHUNKER_H
#define SERVICE_UNIBOARD_CHUNKER_H

#include "../emulators/service/ServiceUdpPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>

using namespace pelican;

class ServiceUniboardChunker : public AbstractChunker
{
public:
    ServiceUniboardChunker(const ConfigNode& inConfig);

    virtual QIODevice* newDevice();
    virtual void next(QIODevice *inDevice);

private:
    qint64 mPacketSize; ///< Size of a udp packet
    static ServiceAntennaUdpPacket sEmptyPacket; ///< Default empty packet
};

PELICAN_DECLARE_CHUNKER(ServiceUniboardChunker)

#endif // SERVICE_UNIBOARD_CHUNKER_H
