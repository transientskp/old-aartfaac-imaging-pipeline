#include "StreamChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QTcpSocket>

StreamChunker::StreamChunker(const ConfigNode &inConfig):
  AbstractChunker(inConfig),
  mPacket(new StreamPacket()),
  mPacketSize(sizeof(StreamPacket)),
  mAllChunksSize(0)
{
  QString s = inConfig.getOption("channel", "subbands");
  mSubbands = ParseSubbands(s);
  std::sort(mSubbands.begin(), mSubbands.end());
  qDebug("Subbands (%ld):", mSubbands.size());
  for (int i = 0, n = mSubbands.size(); i < n; i++)
    qDebug("  (%2d,%2d) bytes(%lu)", mSubbands[i].c1, mSubbands[i].c2, mSubbands[i].size);
  mTimeOut = inConfig.getOption("connection", "timeout", "5000").toInt();
}

StreamChunker::~StreamChunker()
{
  delete mServer;
  delete mPacket;
}

QIODevice *StreamChunker::newDevice()
{
  mServer = new QTcpServer();
  mServer->listen(QHostAddress(host()), port());

  if (!mServer->waitForNewConnection(mTimeOut))
  {
    delete mServer;
    qFatal("Connection timeout after %d msec", mTimeOut);
  }
  return mServer->nextPendingConnection();
}

void StreamChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  std::vector<WritableData> chunks(mSubbands.size());
  std::vector<size_t> bytes(mSubbands.size(), 0);

  for (int b = 0; b < NUM_BASELINES; b++)
  {
    while (inDevice->bytesAvailable() < mPacketSize)
      inDevice->waitForReadyRead(1);
    inDevice->read(reinterpret_cast<char*>(mPacket), mPacketSize);

    if (b == 0)
    {
      for (int i = 0, n = mSubbands.size(); i < n; i++)
      {
        ChunkHeader chunk_header;
        chunk_header.time = mPacket->mHeader.time;
        chunk_header.freq = mPacket->mHeader.freq;
        chunk_header.chan_width = mPacket->mHeader.chan_width;
        chunk_header.start_chan = mSubbands[i].c1;
        chunk_header.end_chan = mSubbands[i].c2;

        chunks[i] = getDataStorage(mSubbands[i].size);
        chunks[i].write(static_cast<void*>(&chunk_header), sizeof(ChunkHeader), bytes[i]);
        bytes[i] += sizeof(ChunkHeader);
      }
    }

    for (int i = 0, n = mSubbands.size(); i < n; i++)
    {
      Subband &s = mSubbands[i];

      for (int c = s.c1; c <= s.c2; c++)
      {
        for (int p = 0; p < NUM_POLARIZATIONS; p++)
        {
          chunks[i].write(static_cast<void*>(&mPacket->visibilities[c][p]),
                          sizeof(std::complex<float>),
                          bytes[i]);
          bytes[i] += sizeof(std::complex<float>);
        }
      }
    }
  }

#ifndef NDEBUG
  for (int i = 0, n = mSubbands.size(); i < n; i++)
    Q_ASSERT(mSubbands[i].size == bytes[i]);
#endif
}

std::vector<StreamChunker::Subband> StreamChunker::ParseSubbands(const QString &s)
{
  std::vector<StreamChunker::Subband> subbands;

  int channel = -1;
  bool success;
  subbands.push_back(Subband());

  for (int i = 0; i < s.size(); )
  {
    QChar c = s.at(i);
    if (c.isDigit())
    {
      int j = i + 1;
      while (j < s.size() && s[j].isDigit())
        j++;

      channel = s.mid(i,j-i).toInt(&success);
      if (!success)
        qFatal("Invalid number %s", qPrintable(s.mid(i,j-i)));
      i += j - i;
    }
    else
    {
      switch (c.toAscii())
      {
      case ',':
        subbands.back().c2 = channel;
        subbands.push_back(Subband());
        break;
      case '-':
        subbands.back().c1 = channel;
        break;
      default: qFatal("Invalid character `%c' at line %d", s[i].toAscii(), i);
      }
      i++;
    }
  }

  subbands.back().c2 = channel;
  for (int i = 0, n = subbands.size(); i < n; i++)
  {
    Subband &s = subbands[i];
    if (s.c1 >= s.c2)
      qFatal("Invalid subband: %d < %d does not hold", s.c1, s.c2);

    s.size = sizeof(ChunkHeader) + (s.c2-s.c1 + 1) * NUM_BASELINES *
             NUM_POLARIZATIONS * sizeof(std::complex<float>);
    mAllChunksSize += s.size;
  }
  return subbands;
}
