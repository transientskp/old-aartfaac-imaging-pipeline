#ifndef STATISTICS_H
#define STATISTICS_H

#include <QtCore>
#include <QtNetwork>
#include <eigen3/Eigen/Dense>

#include <sstream>
#include <iostream>
#include <typeinfo>

#define MAX_ENTRIES 1000
#define STATISTICS_VERSION 1

class Statistics : public QThread
{
Q_OBJECT
public:
  Statistics();
  ~Statistics();

  static Statistics *instance();
  static void destroy();

  void append(const char *name, const char *data);

protected:
  void run();

private:
  QHostAddress mAddress;
  quint16 mPort;
  bool mAbort;
  QList<QString> mQueue;
  QMutex mMutex;
  static Statistics *mInstance;
};

#define ADD_STAT(name, msg)                                 \
  do {                                                      \
    std::stringstream ss;                                   \
    ss.setf(std::ios::scientific);                          \
    ss << msg << std::endl;                                 \
    Statistics::instance()->append(name, ss.str().c_str()); \
  } while (0)

#endif // STATISTICS_H
