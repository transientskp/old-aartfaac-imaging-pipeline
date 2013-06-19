#ifndef UDP_STREAM_PACKET_H
#define UDP_STREAM_PACKET_H

#include <QtCore>
#include <complex>

#include "../../Constants.h"

class StreamPacket
{
public:
  struct Header
  {
    double freq;       ///< frequency in Hz
    double chan_width; ///< Frequency resolution of channels
    double time;       ///< time in MJD (https://en.wikipedia.org/wiki/Julian_date#Alternatives)
    quint16 a1, a2;    ///< The two antennas determining the baseline
    quint16 channels;  ///< Number of channels
  } mHeader;

  std::complex<float> visibilities[NUM_CHANNELS][NUM_POLARIZATIONS];
};

#endif // UDP_STREAM_PACKET_H
