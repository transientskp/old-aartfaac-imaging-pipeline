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
  qCritical("Caught signal %d (%s), shutting down", signal, strsignal(signal));
  QCoreApplication::exit(signal);
}

int main(int argc, char *argv[])
{
  Logger::open("aartfaac-server");
  qInstallMsgHandler(Logger::messageHandler);

  if (argc != 2)
  {
    std::cerr << "Usage: aartfaac-server <config.xml>" << std::endl;
    return EXIT_FAILURE;
  }

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

  int retcode = 1;
  try
  {
    pelican::PelicanServer server(&config);

    //server.setVerbosity(10000);
    server.addStreamChunker("StreamChunker");
//    server.addServiceChunker("ServiceChunker");
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
