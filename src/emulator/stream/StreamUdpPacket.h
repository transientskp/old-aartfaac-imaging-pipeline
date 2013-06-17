#ifndef UDP_STREAM_PACKET_H
#define UDP_STREAM_PACKET_H

#include <QtCore>
#include <complex>

#include "../../Constants.h"

class StreamUdpPacket
{
public:
  struct Header
  {
    double freq;       ///< frequency in Hz
    double chan_width; ///< Frequency resolution of channels
    double time;       ///< time in MJD (https://en.wikipedia.org/wiki/Julian_date#Alternatives)
    quint16 channels;  ///< Number of actual channels
  } mHeader;

  std::complex<float> visibilities[NUM_BASELINES][NUM_CHANNELS][NUM_POLARIZATIONS][NUM_POLARIZATIONS];
};

#endif // UDP_STREAM_PACKET_H
