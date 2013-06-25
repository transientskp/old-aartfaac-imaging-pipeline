#include "StreamChunker.h"

#include "../Constants.h"
#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QTcpSocket>

StreamChunker::StreamChunker(const ConfigNode &config):
  AbstractChunker(config),
  mServer(0),
  mNumChannels(0),
  mFrequency(0.0),
  mFrequencyWidth(0.0)
{
  QString s = config.getOption("channel", "subbands");
  mSubbands = ParseSubbands(s);
  std::sort(mSubbands.begin(), mSubbands.end());

  qDebug("Subbands (%ld):", mSubbands.size());
  for (int i = 0, n = mSubbands.size(); i < n; i++)
    qDebug("  (%d-%d)\t(%d) chunksize %lu bytes",
           mSubbands[i].c1, mSubbands[i].c2, mSubbands[i].channels, mSubbands[i].size);

  mTimeOut = config.getOption("connection", "timeout", "5000").toInt();

  // NOTE: These defaults are associated with SB002_LBA_OUTER_SPREAD.MS.trimmed
  mNumChannels = config.getOption("channel", "amount", "64").toInt();
  mFrequency = config.getOption("channel", "frequency", "54786682.128906").toDouble();
  mFrequencyWidth = config.getOption("channel", "frequency_width", "3051.757812").toDouble();

  qDebug("Channels (%d):", mNumChannels);
  qDebug("  Frequency ref: %f", mFrequency);
  qDebug("  Channel width: %f", mFrequencyWidth);
  mVisibilities = new std::complex<float>[mNumChannels*NUM_POLARIZATIONS];
}

StreamChunker::~StreamChunker()
{
  delete mServer;
  delete[] mVisibilities;
}

QIODevice *StreamChunker::newDevice()
{
  mServer = new QTcpServer();
  mServer->listen(QHostAddress(host()), port());

  if (!mServer->waitForNewConnection(mTimeOut))
  {
    this->~StreamChunker();
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
  ChunkHeader chunk_header;
  StreamHeader stream_header;

  // Wait for enough bytes and parse the streaming header
  qint64 stream_hdr_size = sizeof(StreamHeader);
  while (inDevice->bytesAvailable() < stream_hdr_size)
    inDevice->waitForReadyRead(1);

  inDevice->read(reinterpret_cast<char*>(&stream_header), stream_hdr_size);

  if (stream_header.magic != HEADER_MAGIC)
    qFatal("Invalid packet, magics do not match");

  // Allocate chunk memory for each subband and write chunker header
  chunk_header.time = stream_header.end_time;
  for (int i = 0, n = mSubbands.size(); i < n; i++)
  {
    chunk_header.freq = mFrequency;
    chunk_header.chan_width = mFrequencyWidth;
    chunk_header.start_chan = mSubbands[i].c1;
    chunk_header.end_chan = mSubbands[i].c2;

    chunks[i] = getDataStorage(mSubbands[i].size);
    if (!chunks[i].isValid())
      qFatal("[%s()] Not enough memory", __FUNCTION__);

    chunks[i].write(static_cast<void*>(&chunk_header), sizeof(ChunkHeader), bytes[i]);
    bytes[i] += sizeof(ChunkHeader);
  }

  // Start reading data from device and write to the appropriate chunk/subband
  for (int b = 0; b < NUM_BASELINES; b++)
  {
    qint64 baseline_size = mNumChannels*NUM_POLARIZATIONS*sizeof(std::complex<float>);
    while (inDevice->bytesAvailable() < baseline_size);
      inDevice->waitForReadyRead(10);

    inDevice->read(reinterpret_cast<char*>(mVisibilities), baseline_size);

    for (int i = 0, n = mSubbands.size(); i < n; i++)
    {
      Subband &s = mSubbands[i];

      for (int c = s.c1; c <= s.c2; c++)
      {
        for (int p = 0; p < NUM_POLARIZATIONS; p++)
        {
          chunks[i].write(static_cast<void*>(&mVisibilities[c*NUM_POLARIZATIONS+p]),
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
    s.channels = s.c2 - s.c1 + 1;

    if (s.c1 >= s.c2)
      qFatal("Invalid subband: %d < %d does not hold", s.c1, s.c2);

    if (s.channels > MAX_MERGE_CHANNELS)
      qFatal("Too many channels in single subband: %d <= %d does not hold",
             s.channels, MAX_MERGE_CHANNELS);

    s.size = sizeof(ChunkHeader) + s.channels * NUM_BASELINES *
             NUM_POLARIZATIONS * sizeof(std::complex<float>);
  }
  return subbands;
}
