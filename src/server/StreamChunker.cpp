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
  mSubband(0),
  mStartInterval(0.0)
{
  QString s = config.getOption("stream", "channelranges");
  mMinInterval = config.getOption("stream", "interval", "0").toInt();
  mNumChannels = config.getOption("stream", "numChannels", "63").toInt();
  mSubband = config.getOption("stream", "subband", "296").toInt();

  mChannelRanges = ParseChannels(s);
  std::sort(mChannelRanges.begin(), mChannelRanges.end());

  std::cout << std::endl;
  qDebug("----- Stream (%i) ------", port());
  qDebug("Channel ranges (%ld):", mChannelRanges.size());
  for (int i = 0, n = mChannelRanges.size(); i < n; i++)
    qDebug("  (%02i-%02i) %0.5f [Hz] chunksize %lu bytes", mChannelRanges[i].c1,
           mChannelRanges[i].c2,
           utils::Range2Frequency(mSubband, mChannelRanges[i].c1, mChannelRanges[i].c2 + 1),
            mChannelRanges[i].size);

  qDebug("Channels in stream(%i):", mNumChannels);
  qDebug("  Lofar subband:     %i", mSubband);
  qDebug("  Central frequency: %0.5f [Hz]", utils::Subband2Frequency(mSubband));
  mVisibilities.resize(NUM_BASELINES*mNumChannels*NUM_POLARIZATIONS);
}

StreamChunker::~StreamChunker()
{
  if (mServer)
    delete mServer;
}

QIODevice *StreamChunker::newDevice()
{
  if (mServer != 0)
  {
    mServer->close();
    delete mServer;
  }

  mServer = new QTcpServer();
  mServer->listen(QHostAddress::Any, port());

  mServer->waitForNewConnection(-1);
  qWarning("[%s] Created new connection %s:%d",
         __PRETTY_FUNCTION__, qPrintable(host()), port());
  return mServer->nextPendingConnection();
}

void StreamChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  mTimer.start();

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

  // Allocate chunk memory for each subband and write chunker header
  std::vector<WritableData> chunks(mChannelRanges.size());
  std::vector<size_t> bytes(mChannelRanges.size(), 0);
  ChunkHeader chunk_header;

  chunk_header.time = utils::UnixTime2MJD(stream_header.end_time);
  for (int i = 0, n = mChannelRanges.size(); i < n; i++)
  {
    chunk_header.subband = mSubband;
    chunk_header.start_chan = mChannelRanges[i].c1;
    chunk_header.end_chan = mChannelRanges[i].c2;

    chunks[i] = getDataStorage(mChannelRanges[i].size);
    if (!chunks[i].isValid())
    {
      qCritical("[%s()] Not enough memory", __FUNCTION__);
      return;
    }

    chunks[i].write(static_cast<void*>(&chunk_header), sizeof(ChunkHeader), bytes[i]);
    bytes[i] += sizeof(ChunkHeader);
  }

  // Start reading data from device and write to the appropriate chunk/subband
  int realtime = int(mNumChannels == MAX_MERGE_CHANNELS-1);
  for (int i = 0, n = mChannelRanges.size(); i < n; i++)
  {
    ChannelRange &s = mChannelRanges[i];
    for (int p = XX_POL; p < NUM_POLARIZATIONS; p+=YY_POL)
    {
      for (int b = 0; b < NUM_BASELINES; b++)
      {
        for (int c = s.c1 - realtime; c <= s.c2 - realtime; c++)
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

  mStartInterval = stream_header.start_time;
  float bps = (mVisibilities.size()*sizeof(std::complex<float>)+sizeof(ChunkHeader)) * 8 / (mTimer.elapsed() / 1000.0f);
  qDebug("Stream '%s' %s-%s %0.2f Gb/s", qPrintable(name()),
         qPrintable(QDateTime::fromTime_t(stream_header.start_time).toString("hh:mm:ss")),
         qPrintable(QDateTime::fromTime_t(stream_header.end_time).toString("hh:mm:ss")),
         bps*1e-9f);
}

std::vector<StreamChunker::ChannelRange> StreamChunker::ParseChannels(const QString &s)
{
  std::vector<StreamChunker::ChannelRange> channel_ranges;

  int channel = -1;
  bool success;
  channel_ranges.push_back(ChannelRange());

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
        channel_ranges.back().c2 = channel;
        channel_ranges.push_back(ChannelRange());
        break;
      case '-':
        channel_ranges.back().c1 = channel;
        break;
      default: qFatal("Invalid character `%c' at line %d", s[i].toAscii(), i);
      }
      i++;
    }
  }

  channel_ranges.back().c2 = channel;
  for (int i = 0, n = channel_ranges.size(); i < n; i++)
  {
    ChannelRange &s = channel_ranges[i];
    s.channels = s.c2 - s.c1 + 1;

    if (s.c1 > s.c2)
      qFatal("Invalid subband: %d <= %d does not hold", s.c1, s.c2);

    if (s.channels > MAX_MERGE_CHANNELS)
      qFatal("Too many channels in single subband: %d <= %d does not hold",
             s.channels, MAX_MERGE_CHANNELS);

    s.size = sizeof(ChunkHeader) + s.channels * NUM_BASELINES *
             NUM_USED_POLARIZATIONS * sizeof(std::complex<float>);
  }
  return channel_ranges;
}
