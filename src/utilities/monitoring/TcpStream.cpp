#include <arpa/inet.h>
#include "TcpStream.h"

TCPStream::TCPStream(int sd, struct sockaddr_in *address) : mSocketDescriptor(sd)
{
  char ip[50];
  inet_ntop(PF_INET, (struct in_addr *) & (address->sin_addr.s_addr), ip, sizeof(ip) - 1);
  mIP = ip;
  mPort = ntohs(address->sin_port);
}

TCPStream::~TCPStream()
{
  close(mSocketDescriptor);
}

ssize_t TCPStream::send(const char *buffer, size_t len)
{
  return write(mSocketDescriptor, buffer, len);
}

ssize_t TCPStream::receive(char *buffer, size_t len)
{
  return read(mSocketDescriptor, buffer, len);
}

string TCPStream::ip()
{
  return mIP;
}

int TCPStream::port()
{
  return mPort;
}

