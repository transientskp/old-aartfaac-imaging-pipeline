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

  WritableData chunk = getDataStorage(mChunkSize);

  if (chunk.isValid())
  {
    int bytes_read = 0;
    QUdpSocket *socket = static_cast<QUdpSocket *>(inDevice);

    // Get pointer to start of chunk memory.
    char *ptr = (char *)chunk.ptr();

    // Read datagrams for chunk from the socket.
    while (isActive() && bytes_read < mChunkSize)
    {
      // Read the datagram, but avoid using pendingDatagramSize()
      if (!socket->hasPendingDatagrams())
      {
        // MUST wait for the next datagram
        socket->waitForReadyRead(100);
        continue;
      }

      qint64 max_length = mChunkSize - bytes_read;
      qint64 length = socket->readDatagram(ptr + bytes_read, max_length);
      if (length > 0)
      {
        bytes_read += length;
        qDebug("Bytes read: %d", bytes_read);
      }
    }
  }
  else
    qFatal("Unable to allocate memory!");
}
