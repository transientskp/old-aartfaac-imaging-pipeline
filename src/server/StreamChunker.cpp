#include "StreamChunker.h"

#include "Constants.h"
#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QTcpSocket>

StreamChunker::StreamChunker(const ConfigNode &config):
  AbstractChunker(config),
  mServer(0),
  mNumChannels(0),
  mMinInterval(0),
  mFrequency(0.0),
  mChannelWidth(0.0),
  mStartInterval(0.0)
{
  QString s = config.getOption("stream", "subbands");
  mSubbands = ParseSubbands(s);
  std::sort(mSubbands.begin(), mSubbands.end());

  qDebug("----- Stream (%d) ------", port());
  qDebug("Subbands (%ld):", mSubbands.size());
  for (int i = 0, n = mSubbands.size(); i < n; i++)
    qDebug("  (%2d-%2d)\t(%d) chunksize %lu bytes",
           mSubbands[i].c1, mSubbands[i].c2, mSubbands[i].channels, mSubbands[i].size);

  // NOTE: These defaults are associated with SB002_LBA_OUTER_SPREAD.MS.trimmed
  mMinInterval = config.getOption("stream", "interval", "0").toInt();
  mNumChannels = config.getOption("stream", "numChannels", "1").toInt();
  mFrequency = config.getOption("stream", "frequency", "54873657.226562").toDouble();
  mChannelWidth = config.getOption("stream", "width", "3051.757812").toDouble();

  qDebug("Channels in stream(%d):", mNumChannels);
  qDebug("  Frequency ref: %f", mFrequency);
  qDebug("  Channel width: %f", mChannelWidth);
  mVisibilities.resize(NUM_BASELINES*mNumChannels*NUM_POLARIZATIONS);
}

StreamChunker::~StreamChunker()
{
  delete mServer;
}

QIODevice *StreamChunker::newDevice()
{
  if (mServer == 0)
  {
    mServer = new QTcpServer();
    mServer->listen(QHostAddress::Any, port());
  }

  mServer->waitForNewConnection(-1);
  qWarning("[%s] Created new connection %s:%d",
         __PRETTY_FUNCTION__, qPrintable(host()), port());
  return mServer->nextPendingConnection();
}

void StreamChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  // Wait for enough bytes and parse the streaming header
  StreamHeader stream_header;
  qint64 size = sizeof(StreamHeader);
  while (inDevice->bytesAvailable() < size)
    inDevice->waitForReadyRead(-1);
  inDevice->read(reinterpret_cast<char*>(&stream_header), size);

  // Read in an entire block of visibilities
  size = mVisibilities.size()*sizeof(std::complex<float>);
  while (inDevice->bytesAvailable() < size)
    inDevice->waitForReadyRead(-1);
  inDevice->read(reinterpret_cast<char*>(mVisibilities.data()), size);

  if (stream_header.magic != HEADER_MAGIC)
  {
    qCritical("Invalid packet, magics do not match");
    return;
  }

  // Only process every Nth second
  if (stream_header.start_time - mStartInterval < mMinInterval)
    return;

  mStartInterval = stream_header.start_time;
  qDebug("Stream '%s' %s-%s", qPrintable(name()),
         qPrintable(QDateTime::fromTime_t(stream_header.start_time).toString("hh:mm:ss")),
         qPrintable(QDateTime::fromTime_t(stream_header.end_time).toString("hh:mm:ss")));

  // Allocate chunk memory for each subband and write chunker header
  std::vector<WritableData> chunks(mSubbands.size());
  std::vector<size_t> bytes(mSubbands.size(), 0);
  ChunkHeader chunk_header;

  chunk_header.time = utils::UnixTime2MJD(stream_header.end_time);
  for (int i = 0, n = mSubbands.size(); i < n; i++)
  {
    chunk_header.freq = mFrequency;
    chunk_header.chan_width = mChannelWidth;
    chunk_header.start_chan = mSubbands[i].c1;
    chunk_header.end_chan = mSubbands[i].c2;

    chunks[i] = getDataStorage(mSubbands[i].size);
    if (!chunks[i].isValid())
    {
      qCritical("[%s()] Not enough memory", __FUNCTION__);
      return;
    }

    chunks[i].write(static_cast<void*>(&chunk_header), sizeof(ChunkHeader), bytes[i]);
    bytes[i] += sizeof(ChunkHeader);
  }

  // Start reading data from device and write to the appropriate chunk/subband
  for (int b = 0; b < NUM_BASELINES; b++)
  {
    for (int i = 0, n = mSubbands.size(); i < n; i++)
    {
      Subband &s = mSubbands[i];

      for (int c = s.c1; c <= s.c2; c++)
      {
        for (int p = XX_POL; p < NUM_POLARIZATIONS; p+=YY_POL)
        {
          chunks[i].write(reinterpret_cast<void*>(&mVisibilities[p+c*NUM_POLARIZATIONS+b*NUM_POLARIZATIONS*mNumChannels]),
                          sizeof(std::complex<float>),
                          bytes[i]);

          bytes[i] += sizeof(std::complex<float>);
        }
      }
    }
  }

  // Check buffer size
  float usage = (usedSize() / float(maxBufferSize())) * 100.0f;
  if (usage > 90.0f)
    qCritical("Chunker `%s' is at %0.1f%% of its buffer", qPrintable(name()), usage);
  else
  if (usage > 75.0f)
    qWarning("Chunker `%s' is at %0.1f%% of its buffer", qPrintable(name()), usage);
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

    if (s.c1 > s.c2)
      qFatal("Invalid subband: %d <= %d does not hold", s.c1, s.c2);

    if (s.channels > MAX_MERGE_CHANNELS)
      qFatal("Too many channels in single subband: %d <= %d does not hold",
             s.channels, MAX_MERGE_CHANNELS);

    s.size = sizeof(ChunkHeader) + s.channels * NUM_BASELINES *
             NUM_USED_POLARIZATIONS * sizeof(std::complex<float>);
  }
  return subbands;
}
