#ifndef TCP_ACCEPTOR_H
#define TCP_ACCEPTOR_H

#include <string>
#include <netinet/in.h>
#include "TcpStream.h"

using namespace std;

class TCPAcceptor
{
  int    mSocketDescriptor;
  int    mPort;
  string mAddress;
  bool   mListening;

public:
  TCPAcceptor(int port, const char *address = "");
  ~TCPAcceptor();

  int        start();
  TCPStream *accept();

private:
  TCPAcceptor() {}
};

#endif // TCP_ACCEPTOR_H
