#ifndef UDP_SERVICE_PACKET_H
#define UDP_SERVICE_PACKET_H

#include <QtCore>

#define MAX_ANTENNAS 83
#define MAX_NAME_SIZE 16

class ServiceUdpPacket
{
public:
  struct Header
  {
    quint16 rows; ///< number of actual rows within a packet
    quint16 antennae; ///< number of total antennas
  } __attribute__((packed)) mHeader;

  struct Antenna
  {
    quint16 id;
    char name[MAX_NAME_SIZE];
  } __attribute__((packed)) mAntennas[MAX_ANTENNAS];
};

#endif // UDP_SERVICE_PACKET_H
