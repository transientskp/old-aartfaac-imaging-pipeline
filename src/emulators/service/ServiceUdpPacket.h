#ifndef UDP_SERVICE_PACKET_H
#define UDP_SERVICE_PACKET_H

#include <QtCore>

class ServiceUdpPacket
{
public:
  struct Header {
    int x;
  } __attribute__((packed)) mHeader;
};

#endif // UDP_SERVICE_PACKET_H
