#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>

using namespace std;

class TCPStream
{
public:
  friend class TCPAcceptor;
  friend class TCPConnector;

  ~TCPStream();

  ssize_t send(const char *buffer, size_t len);
  ssize_t receive(char *buffer, size_t len);

  string ip();
  int    port();

private:
  TCPStream(int sd, struct sockaddr_in *address);
  TCPStream();
  TCPStream(const TCPStream &stream);

  int     mSocketDescriptor;
  string  mIP;
  int     mPort;
};

#endif // TCP_STREAM_H
