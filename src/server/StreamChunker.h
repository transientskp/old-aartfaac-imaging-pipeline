#ifndef STREAM_CHUNKER_H
#define STREAM_CHUNKER_H

#include "../emulator/stream/StreamPacket.h"

#include <pelican/server/AbstractChunker.h>
#include <QtCore>
#include <QtNetwork/QTcpServer>
#include <utility>

using namespace pelican;

class StreamChunker : public AbstractChunker
{
public:
  StreamChunker(const ConfigNode &inConfig);
  ~StreamChunker();

  QIODevice *newDevice();
  void next(QIODevice *inDevice);

private:
  class Subband
  {
  public:
    quint16 c1;
    quint16 c2;
    quint16 channels;
    size_t size;

    bool operator < (const Subband &s) const
    {
      return c2 < s.c1;
    }
  };

  std::vector<Subband> mSubbands;

  QTcpServer *mServer;
  StreamPacket *mPacket;

  int mPacketSize;
  int mTimeOut;

  /** @brief Parses subbands of the form: "c_{1}-c_{2},...,c_{n-1}-c_{n}",
   *         where c_{i} in {0,...,n} and c_{i} < c_{i+1}
   */
  std::vector<Subband> ParseSubbands(const QString &s);
};

PELICAN_DECLARE_CHUNKER(StreamChunker)

#endif // STREAM_CHUNKER_H
