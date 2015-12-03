#ifndef STREAM_CHUNKER_H
#define STREAM_CHUNKER_H

#include "../emulator/stream/StreamPacket.h"

#include <complex>
#include <pelican/server/AbstractChunker.h>
#include <QtCore>
#include <QtNetwork/QTcpServer>
#include <utility>
#include <vector>

using namespace pelican;

class StreamChunker : public AbstractChunker
{
public:
  StreamChunker(const ConfigNode &config);
  ~StreamChunker();

  QIODevice *newDevice();
  void next(QIODevice *inDevice);

private:
  class ChannelRange
  {
  public:
    quint16 c1;
    quint16 c2;
    quint16 channels;
    size_t size;

    bool operator < (const ChannelRange &s) const
    {
      return c2 < s.c1;
    }
  };

  std::vector<std::complex<float> > mVisibilities;
  QTcpServer *mServer; ///< TCP/IP server, created in newDevice()

  // Must be defined in serverConf.xml
  std::vector<ChannelRange> mChannelRanges;
  int mNumChannels;
  int mMinInterval;
  int mSubband;
  double mStartInterval;
  QTime mTimer;

  /** @brief Parses subbands of the form: "c_{1}-c_{2},...,c_{n-1}-c_{n}",
   *         where c_{i} in {0,...,n} and c_{i} < c_{i+1}
   */
  std::vector<ChannelRange> ParseSubbands(const QString &s);
};

PELICAN_DECLARE_CHUNKER(StreamChunker)

#endif // STREAM_CHUNKER_H
