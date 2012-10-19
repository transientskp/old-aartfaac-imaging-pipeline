#include "ServiceUniboardChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QUdpSocket>

ServiceAntennaUdpPacket ServiceUniboardChunker::sEmptyPacket;

ServiceUniboardChunker::ServiceUniboardChunker(const ConfigNode &inConfig)
  : AbstractChunker(inConfig)
{
  mPacketSize = sizeof(sEmptyPacket);
}

QIODevice* ServiceUniboardChunker::newDevice()
{
  QUdpSocket *socket = new QUdpSocket();
  if (!socket->bind(QHostAddress(host()), port()))
    qFatal("Failed to connect to %s:%u", qPrintable(host()), port());

  while (socket->state() != QUdpSocket::BoundState) {}
  return socket;
}

void ServiceUniboardChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  quint64 bytes_received = 0;

  QUdpSocket *udp_socket = static_cast<QUdpSocket*>(inDevice);

  ServiceAntennaUdpPacket packet;

  while (inDevice->bytesAvailable() < mPacketSize)
    udp_socket->waitForReadyRead(100);

  if (udp_socket->readDatagram(reinterpret_cast<char*>(&packet), mPacketSize) <= 0)
    qWarning("Failed receiving UDP packet");
  else
    qDebug("Antenna pos: %g %g %g", packet.mAntennas[0].pos[0],
                                    packet.mAntennas[0].pos[1],
                                    packet.mAntennas[0].pos[2]);
  bytes_received += mPacketSize;
}
