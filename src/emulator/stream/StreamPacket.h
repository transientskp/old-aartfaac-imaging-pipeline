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
    double freq;       ///< Frequency in Hz
    double chan_width; ///< Frequency resolution of channels
    double time;       ///< time in MJD
  } mHeader;

  std::complex<float> visibilities[NUM_CHANNELS][NUM_POLARIZATIONS];
};

struct ChunkHeader
{
  double freq;
  double chan_width;
  double time;
  quint16 start_chan;
  quint16 end_chan;
};

#endif // UDP_STREAM_PACKET_H
