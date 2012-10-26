#include "ServiceChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

ServiceChunker::ServiceChunker(const ConfigNode &inConfig)
  : AbstractChunker(inConfig),
  mBytesReceived(0)
{
  mPacketSize = sizeof(ServiceUdpPacket);
  mAntennas = inConfig.getOption("antennae", "count").toInt();
  mChunkSize = ceil(mAntennas / double(MAX_ANTENNAS)) * mPacketSize;
  Q_ASSERT(mChunkSize % mPacketSize == 0);
}

QIODevice *ServiceChunker::newDevice()
{
  QUdpSocket *socket = new QUdpSocket();

  if (!socket->bind(QHostAddress(host()), port()))
    qFatal("Failed to connect to %s:%u", qPrintable(host()), port());

  while (socket->state() != QUdpSocket::BoundState)
    {}

  return socket;
}

void ServiceChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  if (mBytesReceived == 0)
  {
    mChunk = new WritableData();
    *mChunk = getDataStorage(mChunkSize);
    Q_ASSERT(mChunk->isValid());
  }

  QUdpSocket *udp_socket = static_cast<QUdpSocket *>(inDevice);

  ServiceUdpPacket packet;

  while (inDevice->bytesAvailable() < mPacketSize)
    udp_socket->waitForReadyRead(-1);

  if (udp_socket->readDatagram(reinterpret_cast<char *>(&packet), mPacketSize) <= 0)
    qWarning("Failed receiving UDP packet");
  else
  {
    Q_ASSERT(mChunk != NULL);
    mChunk->write(static_cast<void *>(&packet), mPacketSize, mBytesReceived);
    mBytesReceived += mPacketSize;
    qWarning("BYTES: %lld", mBytesReceived);
  }

  if (mBytesReceived == mChunkSize)
  {
    qCritical("DELETING!");
    delete mChunk;
    mChunk = NULL;
    mBytesReceived = 0;
  }
}
