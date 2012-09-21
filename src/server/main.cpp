#include <pelican/server/PelicanServer.h>
#include <pelican/comms/PelicanProtocol.h>
#include <pelican/utility/Config.h>

#include "../utilities/Logger.h"
#include "version.h"

#include <iostream>
#include <QtCore>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: aartfaac-server <config.xml>" << std::endl;
    return EXIT_FAILURE;
  }

  Logger::open(NAME"-server");
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  QString config_file(argv[1]);
  pelican::Config config(config_file);

  qDebug("%s", HUMAN_NAME);
  try
  {
    pelican::PelicanServer server(&config);

    server.addStreamChunker("UniboardChunker");

    server.addProtocol(new pelican::PelicanProtocol(), 2000);

    server.start();

    while (!server.isReady()) {}

    return app.exec();
  }
  catch (const QString& error)
  {
    qFatal("%s", qPrintable(error));
  }
}
