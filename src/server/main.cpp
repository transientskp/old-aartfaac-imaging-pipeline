#include <pelican/server/PelicanServer.h>
#include <pelican/comms/PelicanProtocol.h>
#include <pelican/utility/Config.h>

#include "../utilities/Logger.h"
#include "version.h"

#include <csignal>
#include <iostream>
#include <QtCore>

void sighandler(int signal)
{
  qCritical("Received signal %d (%s), exit now", signal, strsignal(signal));
  exit(signal);
}

void usage(int signal)
{
  std::cout << "Usage: aartfaac-server <XML>" << std::endl;
  std::cout << " Starts an aartfaac server." << std::endl;
  std::cout << " Configurable through xml file." << std::endl;
  std::cout << " Example: aartfaac-server server.xml" << std::endl << std::endl;

  std::cout << " XML\tlocation of the xml config file" << std::endl;
  exit(signal);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    usage(EXIT_FAILURE);

  Logger::open("aartfaac-server");
  qInstallMsgHandler(Logger::messageHandler);

  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  signal(SIGQUIT, &sighandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  QString config_file(argv[1]);
  pelican::Config config(config_file);

  std::cout << HUMAN_NAME << std::endl;
  pelican::Config::TreeAddress address;
  address << pelican::Config::NodeId("configuration","")
          << pelican::Config::NodeId("server", "")
          << pelican::Config::NodeId("chunkers", "");

  pelican::ConfigNode chunker = config.get(address);
  QList<pelican::ConfigNode> streams = chunker.getNodes("StreamChunker");

  int retcode = 1;
  try
  {
    pelican::PelicanServer server(&config);

    //server.setVerbosity(10000);
    for (int i = 0; i < streams.count(); i++)
      server.addStreamChunker("StreamChunker", streams.at(i).name());

    server.addProtocol(new pelican::PelicanProtocol(), 2000);
    server.start();

    while (!server.isReady())
      {}

    retcode = app.exec();
  }
  catch (const QString &error)
  {
    qFatal("%s", qPrintable(error));
  }

  Logger::close();

  return retcode;
}
