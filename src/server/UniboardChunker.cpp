#include "UniboardChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

UdpPacket UniboardChunker::sEmptyPacket;

UniboardChunker::UniboardChunker(const ConfigNode &inConfig)
  : AbstractChunker(inConfig)
{
  sEmptyPacket.mHeader.channel = -1;
  sEmptyPacket.mHeader.samples = 0;
  sEmptyPacket.mHeader.time    = 0.0;
  memset(sEmptyPacket.mSamples, 0, sizeof(sEmptyPacket.mSamples));

  // chunksize = ceil(baselines/samples in packet) * packet size
  mChunkSize = inConfig.getOption("data", "chunkSize").toUInt();
  mTimeout = inConfig.getOption("chunk", "timeout").toInt();
  mPacketSize = sizeof(UdpPacket);

  Q_ASSERT(mChunkSize % mPacketSize == 0);
}

QIODevice* UniboardChunker::newDevice()
{
  QUdpSocket *socket = new QUdpSocket();
  if (!socket->bind(QHostAddress(host()), port()))
    qFatal("Failed to connect to %s:%u", qPrintable(host()), port());

  while (socket->state() != QUdpSocket::BoundState) {}
  return socket;
}

void UniboardChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  QUdpSocket *udp_socket = static_cast<QUdpSocket*>(inDevice);

  quint32 packets = mChunkSize / mPacketSize;
  UdpPacket packet;
  for (quint32 i = 0; i < packets; i++)
  {
    while (inDevice->bytesAvailable() < mPacketSize)
      udp_socket->waitForReadyRead(100);

    if (udp_socket->readDatagram(reinterpret_cast<char*>(&packet), mPacketSize) <= 0)
    {
      qWarning("Failed receiving UDP packet %u/%u", i, packets);
      i--;
      continue;
    }

    QString key = hash(packet.mHeader.time, packet.mHeader.channel);
    if (!mDataBuffers.contains(key))
      mDataBuffers[key] = new Chunk(this);

    mDataBuffers[key]->addData(static_cast<void*>(&packet), mPacketSize);
  }

  QHash<QString, Chunk*>::iterator i = mDataBuffers.begin();
  Chunk *chunk;
  while (i != mDataBuffers.end())
  {
    chunk = i.value();
    if (chunk->isFilled())
    {
      delete chunk;
      i = mDataBuffers.erase(i);
    }
    else
    if (chunk->isTimeUp())
    {
      quint32 missing_bytes = chunk->fill();
      qWarning("Chunk '%s' is incomplete, missing %u/%lld bytes",
               qPrintable(i.key()), missing_bytes, mChunkSize);
      delete chunk;
      i = mDataBuffers.erase(i);
    }
    else
    {
      ++i;
    }
  }
}

QString UniboardChunker::hash(const double inTime, const quint32 inChannelId)
{

  return QString("T:%1 C:%2").arg(utils::MJD2QDateTime(inTime).toString("dd-MM-yyyy hh:mm:ss")).arg(inChannelId);
}

UniboardChunker::Chunk::Chunk(UniboardChunker *inChunker)
  : mBytesRead(0),
    mChunker(inChunker)
{
  mData = mChunker->getDataStorage(mChunker->mChunkSize);
  if (!mData.isValid())
    qCritical("Unable to allocate chunk memory, is the buffer large enough?");
  mPtr = (char*) mData.ptr();
  mTimer.start();
}

bool UniboardChunker::Chunk::isTimeUp()
{
  return mTimer.elapsed() >= mChunker->mTimeout;
}

bool UniboardChunker::Chunk::isFilled()
{
  return mBytesRead >= mChunker->mChunkSize;
}

void UniboardChunker::Chunk::addData(const void *inData, const int inLength)
{
  mData.write(inData, inLength, mBytesRead);
  mBytesRead += inLength;
}

quint32 UniboardChunker::Chunk::fill()
{
  quint32 bytes_left = mChunker->mChunkSize - mBytesRead;
  Q_ASSERT(bytes_left % sizeof(UdpPacket) == 0);

  for (quint32 i = 0, n = bytes_left / sizeof(UdpPacket); i < n; i++)
  {
    mData.write(static_cast<void*>(&sEmptyPacket), sizeof(UdpPacket), mBytesRead);
    mBytesRead += sizeof(UdpPacket);
  }

  return bytes_left;
}
