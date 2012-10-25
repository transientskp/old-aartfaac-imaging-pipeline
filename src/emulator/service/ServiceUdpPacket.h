#ifndef UDP_SERVICE_PACKET_H
#define UDP_SERVICE_PACKET_H

#include <QtCore>

#define MAX_ANTENNAS 8

class ServiceAntennaUdpPacket
{
public:
  struct Header
  {
    quint32 rows; ///< number of actual rows within a packet
  } __attribute__((packed)) mHeader;

  struct Antenna
  {
    double offset[3];
    double pos[3];
    char antenna_type[16];
    double dish_diameter;
    bool flag_row;
    char mount[16];
    char name[16];
    char station[16];
    quint32 station_id;
    double phase_ref[3];
  } __attribute__((packed)) mAntennas[MAX_ANTENNAS];
};

#endif // UDP_SERVICE_PACKET_H
