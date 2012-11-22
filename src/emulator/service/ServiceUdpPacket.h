#ifndef UDP_SERVICE_PACKET_H
#define UDP_SERVICE_PACKET_H

#include <QtCore>

#include "../../Constants.h"

#define MAX_ANTENNAS 83

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
    char name[MAX_CHARS_ANTENNA_NAME];
  } __attribute__((packed)) mAntennas[MAX_ANTENNAS];
};

#endif // UDP_SERVICE_PACKET_H
