#include "DataChunker.h"
#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

DataChunker::DataChunker(const pelican::ConfigNode &config)
{
  _chunkSize = config.getOption("data", "chunkSize").toInt();
}

QIODevice* DataChunker::newDevice()
{
  QUdpSocket *socket = new QUdpSocket();
  socket->bind(QHostAddress(host()), port());

  while (socket->state() != QUdpSocket::BoundState) {}
  return socket;
}

void DataChunker::next(QIODevice *device)
{
  QUdpSocket *udpSocket = static_cast<QUdpSocket*>(device);
  _bytesRead = 0;

  pelican::WritableData writableData = getDataStorage(_chunkSize);
  if (writableData.isValid())
  {
    char *ptr = (char*) writableData.ptr();

    while (isActive() && _bytesRead < _chunkSize)
    {
      if (!udpSocket->hasPendingDatagrams())
      {
        udpSocket->waitForReadyRead(100);
        continue;
      }

      qint64 maxlen = _chunkSize - _bytesRead;
      qint64 length = udpSocket->readDatagram(ptr + _bytesRead, maxlen);
      if (length > 0)
        _bytesRead += length;
    }
  }
  else
  {
    udpSocket->readDatagram(0, 0);
  }
}
