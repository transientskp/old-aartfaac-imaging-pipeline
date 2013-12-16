#ifndef SERVER_H
#define SERVER_H

#include <list>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>

#include "Thread.h"
#include "TcpAcceptor.h"
#include "TcpStream.h"

using namespace std;

#define PROTOCOL_VERSION 0

class Client : public Thread
{
public:
  Client(TCPStream *stream);
  ~Client();
  void addMessage(const string &msg);
  bool isConnected();

private:
  void *run();
  list<string> mMessages;
  TCPStream *mStream;
  volatile bool mConnected;
  volatile bool mRunning;
  pthread_mutex_t mMutex;
};

class Server : public Thread
{
public:
  Server(const int port, const char *address = "");
  ~Server();

  void broadcast(const string &msg);
  static Server *Instance();

private:
  void *run();

  volatile bool mIsRunning;
  size_t mMaxConnections;

  pthread_mutex_t mMutex;
  TCPAcceptor *mAcceptor;
  list<Client*> mClients;

  static Server *sInstance;
};

#define ADD_STAT(name, time, data)               \
  do {                                           \
    std::stringstream ss;                        \
    ss << PROTOCOL_VERSION << " " << name << " ";\
    ss.setf(std::ios::fixed);                    \
    ss << std::setprecision(4) << time << " ";   \
    ss.setf(std::ios::scientific);               \
    ss << std::setprecision(5) << data;         \
    Server::Instance()->broadcast(ss.str());     \
  } while (0)

#endif // SERVER_H
