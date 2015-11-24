#ifndef UDP_STREAM_PACKET_H
#define UDP_STREAM_PACKET_H

#include <QtCore>
#include <complex>

#include "Constants.h"

struct StreamHeader
{
  quint32 magic;     ///< magic to determine header
  quint32 pad0;      ///< padding 0
  double start_time; ///< time in MJD
  double end_time;   ///< end time in MJD
  char pad1[512-24]; ///< padding 1, will define a 512 byte block
};

struct ChunkHeader
{
  double time;        ///< time in MJD
  quint32 subband;    ///< LOFAR subband
  quint16 start_chan; ///< Start channel
  quint16 end_chan;   ///< End channel
};

#endif // UDP_STREAM_PACKET_H
