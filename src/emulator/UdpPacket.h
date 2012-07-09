#ifndef UDP_PACKET_H
#define UDP_PACKET_H

#include <QtCore>

#define MAX_CORRELATIONS 200

class UdpPacket
{
public:
  struct Header {
    quint32 correlations; ///< number of actual correlations within a packet
    double freq; ///< frequency in Hz
    double time; ///< time in MJD (https://en.wikipedia.org/wiki/Julian_date#Alternatives)
  } __attribute__((packed)) mHeader;

  struct Correlation {
    quint16 a1; ///< antenna1 casacore id
    quint16 a2; ///< antenna2 casacore id
<<<<<<< HEAD
    float polarizations[8]; ///< single precision xx, yy, xy, yx polarizations in [re_{xx},im_{xx},...,re_{yx},im_{yx}]
  } __attribute__((packed)) mCorrelations[74];
=======
    quint16 polarizations[8]; ///< half precision xx, yy, xy, yx polarizations in [re_{xx},im_{xx},...,re_{yx},im_{yx}]
  } __attribute__((packed)) mCorrelations[MAX_CORRELATIONS];
>>>>>>> server
};

#endif // UDP_PACKET_H
