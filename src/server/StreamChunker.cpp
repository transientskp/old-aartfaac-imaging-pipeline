#include "StreamChunker.h"

#include "../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtNetwork/QTcpSocket>

StreamChunker::StreamChunker(const ConfigNode &inConfig):
  AbstractChunker(inConfig)
{
  QString s = inConfig.getOption("channel", "subbands");
  mSubbands = ParseSubbands(s);
  std::sort(mSubbands.begin(), mSubbands.end());
  qDebug("Subbands:");
  for (int i = 0, n = mSubbands.size(); i < n; i++)
    qDebug("<%d,%d>", mSubbands[i].first, mSubbands[i].second);
  mPacketSize = sizeof(StreamPacket);
  mTimeOut = inConfig.getOption("connection", "timeout", "5000").toInt();
  mPacket = new StreamPacket();
}

StreamChunker::~StreamChunker()
{
  delete mServer;
  delete mPacket;
}

QIODevice *StreamChunker::newDevice()
{
  mServer = new QTcpServer();
  mServer->listen(QHostAddress(host()), port());

  if (!mServer->waitForNewConnection(mTimeOut))
  {
    delete mServer;
    qFatal("Connection timeout after %d msec", mTimeOut);
  }
  return mServer->nextPendingConnection();
}

void StreamChunker::next(QIODevice *inDevice)
{
  if (!isActive())
    return;

  while (inDevice->bytesAvailable() < mPacketSize)
    inDevice->waitForReadyRead(100);

  inDevice->read(reinterpret_cast<char*>(mPacket), mPacketSize);

  qDebug("time: %s", qPrintable(utils::MJD2QDateTime(mPacket->mHeader.time).toString("hh:mm:ss")));
}

std::vector<StreamChunker::Subband> StreamChunker::ParseSubbands(const QString &s)
{
  std::vector<StreamChunker::Subband> subbands;

  int channel = -1;
  bool success;
  subbands.push_back(std::make_pair(0, 0));

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
        subbands.back().second = channel;
        subbands.push_back(std::make_pair(0, 0));
        break;
      case '-':
        subbands.back().first = channel;
        break;
      default: qFatal("Invalid character `%c' at line %d", s[i].toAscii(), i);
      }
      i++;
    }
  }

  subbands.back().second = channel;
  for (int i = 0, n = subbands.size(); i < n; i++)
    if (subbands[i].first >= subbands[i].second)
      qFatal("Invalid subband: %d < %d does not hold", subbands[i].first, subbands[i].second);
  return subbands;
}
