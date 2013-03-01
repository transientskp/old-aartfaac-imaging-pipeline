#ifndef UDP_STREAM_PACKET_H
#define UDP_STREAM_PACKET_H

#include <QtCore>

#define MAX_CORRELATIONS 200

class StreamUdpPacket
{
public:
  struct Header
  {
    quint32 correlations; ///< number of actual correlations within a packet
    double freq; ///< frequency in Hz
    double time; ///< time in MJD (https://en.wikipedia.org/wiki/Julian_date#Alternatives)
  } __attribute__((packed)) mHeader;

  struct Correlation
  {
    quint16 a1; ///< antenna1 casacore id
    quint16 a2; ///< antenna2 casacore id
    float polarizations[8]; ///< xx, yy, xy, yx polarizations in [re_{xx},im_{xx},...,re_{yx},im_{yx}]
  } __attribute__((packed)) mCorrelations[MAX_CORRELATIONS];
};

#endif // UDP_STREAM_PACKET_H
