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

  virtual QIODevice *newDevice();
  virtual void next(QIODevice *inDevice);

private:
  typedef std::pair<int,int> Subband;

  std::vector<Subband> ParseSubbands(const QString &s);
  std::vector<Subband> mSubbands;

  QTcpServer *mServer;
  StreamPacket *mPacket;

  int mPacketSize;
  int mTimeOut;
};

PELICAN_DECLARE_CHUNKER(StreamChunker)

#endif // STREAM_CHUNKER_H
