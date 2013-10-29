#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "TcpAcceptor.h"

TCPAcceptor::TCPAcceptor(int port, const char *address):
  mSocketDescriptor(0),
  mPort(port),
  mAddress(address),
  mListening(false) {}

TCPAcceptor::~TCPAcceptor()
{
  if (mSocketDescriptor > 0)
    close(mSocketDescriptor);
}

int TCPAcceptor::start()
{
  if (mListening == true)
    return 0;

  mSocketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address;

  memset(&address, 0, sizeof(address));
  address.sin_family = PF_INET;
  address.sin_port = htons(mPort);

  if (mAddress.size() > 0)
    inet_pton(PF_INET, mAddress.c_str(), &(address.sin_addr));
  else
    address.sin_addr.s_addr = INADDR_ANY;

  int optval = 1;
  setsockopt(mSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  int result = bind(mSocketDescriptor, (struct sockaddr *)&address, sizeof(address));

  if (result != 0)
  {
    perror("bind() failed");
    return result;
  }

  result = listen(mSocketDescriptor, 5);

  if (result != 0)
  {
    perror("listen() failed");
    return result;
  }

  mListening = true;
  return result;
}

TCPStream *TCPAcceptor::accept()
{
  if (mListening == false)
    return NULL;

  struct sockaddr_in address;

  socklen_t len = sizeof(address);

  memset(&address, 0, sizeof(address));

  int sd = ::accept4(mSocketDescriptor, (struct sockaddr *)&address, &len, SOCK_NONBLOCK);

  if (sd < 0)
  {
    perror("accept() failed");
    return NULL;
  }

  return new TCPStream(sd, &address);
}
