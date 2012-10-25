#include "ServiceChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

ServiceAntennaUdpPacket ServiceChunker::sEmptyPacket;

ServiceChunker::ServiceChunker(const ConfigNode &inConfig)
  : AbstractChunker(inConfig)
{
  mPacketSize = sizeof(sEmptyPacket);
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

  quint64 bytes_received = 0;

  QUdpSocket *udp_socket = static_cast<QUdpSocket *>(inDevice);

  ServiceAntennaUdpPacket packet;

  while (inDevice->bytesAvailable() < mPacketSize)
    udp_socket->waitForReadyRead(-1);

  WritableData data = getDataStorage(sizeof(double) * 3);

  if (data.isValid())
  {

    if (udp_socket->readDatagram(reinterpret_cast<char *>(&packet), mPacketSize) <= 0)
      qWarning("Failed receiving UDP packet");
    else
    {

      qDebug("Antenna pos: %g %g %g", packet.mAntennas[0].pos[0],
             packet.mAntennas[0].pos[1],
             packet.mAntennas[0].pos[2]);
      data.write(static_cast<void *>(&packet.mAntennas[0].pos[0]), sizeof(double) * 3);
    }
  }
  else
  {
    qCritical("Could not allocate datastorage");
    udp_socket->readDatagram(0, 0);
  }

  bytes_received += mPacketSize;
}
