#include "DataChunker.h"
#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

DataChunker::DataChunker(const pelican::ConfigNode &inConfig)
{
  mChunkSize = inConfig.getOption("data", "chunkSize").toInt();
}

QIODevice* DataChunker::newDevice()
{
  QUdpSocket *socket = new QUdpSocket();
  socket->bind(QHostAddress(host()), port());

  while (socket->state() != QUdpSocket::BoundState) {}
  return socket;
}

void DataChunker::next(QIODevice *inDevice)
{
  QUdpSocket *udp_socket = static_cast<QUdpSocket*>(inDevice);
  mBytesRead = 0;

  pelican::WritableData writableData = getDataStorage(mChunkSize);
  if (writableData.isValid())
  {
    char *ptr = (char*) writableData.ptr();

    while (isActive() && mBytesRead < mChunkSize)
    {
      if (!udp_socket->hasPendingDatagrams())
      {
        udp_socket->waitForReadyRead(100);
        continue;
      }

      qint64 maxlen = mChunkSize - mBytesRead;
      qint64 length = udp_socket->readDatagram(ptr + mBytesRead, maxlen);
      if (length > 0)
        mBytesRead += length;
    }
  }
  else
  {
    udp_socket->readDatagram(0, 0);
  }
}
