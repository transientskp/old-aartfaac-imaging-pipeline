#include "ServiceChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

ServiceChunker::ServiceChunker(const ConfigNode &inConfig)
  : AbstractChunker(inConfig),
  mBytesReceived(0),
  mRowsReceived(0)
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

  mChunk = new WritableData();
  *mChunk = getDataStorage(mChunkSize);
  Q_ASSERT(mChunk->isValid());
  return socket;
}

void ServiceChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  QUdpSocket *udp_socket = static_cast<QUdpSocket *>(inDevice);

  ServiceUdpPacket packet;

  while (inDevice->bytesAvailable() < mPacketSize)
    udp_socket->waitForReadyRead(-1);

  if (udp_socket->readDatagram(reinterpret_cast<char *>(&packet), mPacketSize) <= 0)
    qWarning("Failed receiving UDP packet");
  else
  {
    for (int i = 0; i < packet.mHeader.rows; i++)
    {
      qDebug("Antenna[%d]: %s", packet.mAntennas[i].id, packet.mAntennas[i].name);
      int bytes = sizeof(packet.mAntennas[i]);
      mChunk->write(static_cast<void *>(&packet.mAntennas[i]), bytes, mBytesReceived);
      mBytesReceived += bytes;
      mRowsReceived++;
    }
  }

  if (mRowsReceived == mAntennas)
  {
    delete mChunk;
    mBytesReceived = 0;
    mRowsReceived = 0;
  }
}
