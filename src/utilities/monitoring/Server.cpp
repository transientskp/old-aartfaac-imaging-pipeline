#include "Server.h"
#include <QDebug>

Server *Server::sInstance = NULL;

void Server::CreateInstance(const int port, const char *address)
{
  if (sInstance == NULL)
    sInstance = new Server(port, address);
  qDebug("Created monitoring server at port %d", port);
}

Server *Server::Instance()
{
  if (sInstance == NULL)
    qFatal("No server instance found");

  return sInstance;
}

Client::Client(TCPStream *stream):
  mStream(stream),
  mConnected(true),
  mRunning(true)
{
  pthread_mutex_init(&mMutex, NULL);
  start();
  printf("Starting client %lu\n", self());
}

Client::~Client()
{
  pthread_mutex_lock(&mMutex);
  mRunning = false;
  pthread_mutex_unlock(&mMutex);
  join();
  delete mStream;
  pthread_mutex_destroy(&mMutex);
}

void *Client::run()
{
  bool connected = true, running = true;
  char buffer[256];

  while (connected && running)
  {
    connected = mStream->receive(buffer, 256) != 0;
    pthread_mutex_lock(&mMutex);

    if (connected)
    {
      if (!mMessages.empty())
      {
        string msg = mMessages.front();
        mStream->send(msg.c_str(), msg.size());
        mMessages.pop_front();
      }
    }

    running = mRunning;
    mConnected = connected;
    pthread_mutex_unlock(&mMutex);
    usleep(1000);
  }

  printf("Client %lu disconnected\n", self());

  return NULL;
}

bool Client::isConnected()
{
  bool connected;
  pthread_mutex_lock(&mMutex);
  connected = mConnected;
  pthread_mutex_unlock(&mMutex);
  return connected;
}

void Client::addMessage(const string &msg)
{
  pthread_mutex_lock(&mMutex);
  mMessages.push_back(msg);
  pthread_mutex_unlock(&mMutex);
}

Server::Server(int port, const char *address):
  mIsRunning(true),
  mMaxConnections(5)
{
  pthread_mutex_init(&mMutex, NULL);
  mAcceptor = new TCPAcceptor(port, address);
  start();
}

Server::~Server()
{
  pthread_mutex_lock(&mMutex);
  mIsRunning = false;
  pthread_mutex_unlock(&mMutex);

  for (list<Client *>::iterator i = mClients.begin(); i != mClients.end(); i++)
    delete (*i);

  mClients.clear();
  join();

  pthread_mutex_destroy(&mMutex);
  delete mAcceptor;
}

void Server::broadcast(const string &msg)
{
  list<Client *>::iterator i;
  Client *client;

  pthread_mutex_lock(&mMutex);

  for (i = mClients.begin(); i != mClients.end(); ++i)
  {
    client = *i;

    if (!client->isConnected())
    {
      i = mClients.erase(i);
      delete client;
      printf("Clients: %lu\n", mClients.size());
      continue;
    }

    client->addMessage(msg + "\n");
  }

  pthread_mutex_unlock(&mMutex);
}

void *Server::run()
{
  if (mAcceptor->start() != 0)
  {
    printf("Error: Could not start server\n");
    exit(1);
  }

  printf("Starting server %lu\n", self());

  bool running = true;
  TCPStream *stream;

  while (running)
  {
    pthread_mutex_lock(&mMutex);
    printf("Clients: %lu\n", mClients.size());
    running = mIsRunning;
    pthread_mutex_unlock(&mMutex);

    stream = mAcceptor->accept();

    if (!stream)
      continue;

    pthread_mutex_lock(&mMutex);

    if (mClients.size() < mMaxConnections)
    {
      printf("Client connected\n");
      mClients.push_back(new Client(stream));
    }
    else
    {
      printf("Error: Maximum number of clients connected\n");
      delete stream;
    }

    pthread_mutex_unlock(&mMutex);
  }

  return NULL;
}
