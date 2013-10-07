#include "Statistics.h"

Statistics *Statistics::mInstance = NULL;

Statistics::Statistics() : QThread()
{
  mQueue.reserve(MAX_ENTRIES);
  mAddress = QHostAddress("127.0.0.1");
  mPort = 5000;
  start(QThread::LowPriority);
}

Statistics::~Statistics()
{
  mAbort = true;
  wait();
}

Statistics* Statistics::instance()
{
  if (mInstance == NULL)
    mInstance = new Statistics();
  return mInstance;
}

void Statistics::destroy()
{
  delete mInstance;
}

void Statistics::append(const char *name, const char *data)
{
  QMutexLocker locker(&mMutex);

  if (mQueue.size() >= MAX_ENTRIES)
    mQueue.removeFirst();

  mQueue.append(QString("%1 %2 %3").arg(STATISTICS_VERSION).arg(name).arg(data));
}

void Statistics::run()
{
  try
  {
    QTcpSocket socket;
    QString s;
    socket.connectToHost(mAddress, mPort);
    socket.waitForConnected(-1);

    while (!mAbort)
    {
      while (!mQueue.empty())
      {
        QMutexLocker locker(&mMutex);

        s = mQueue.takeFirst();
        socket.write(qPrintable(s), s.size());
        while (socket.bytesToWrite() > 0)
          socket.waitForBytesWritten(100);
      }

      msleep(1);
    }
  }
  catch (QString &e)
  {
    qFatal("Error: %s", qPrintable(e));
  }
}
