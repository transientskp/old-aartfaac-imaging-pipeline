#include <pelican/server/PelicanServer.h>
#include <pelican/comms/PelicanProtocol.h>
#include <pelican/utility/Config.h>

#include "../utilities/Logger.h"
#include "version.h"

#include <QtCore>

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: aartfaac-server <config.xml>" << std::endl;
    return EXIT_FAILURE;
  }

  Logger::setLogFileProperties(NAME"-server.log", 10, 1024*1024*10);
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  QString configFile(argv[1]);
  pelican::Config config(configFile);

  try
  {
    pelican::PelicanServer server(&config);

    server.addStreamChunker("DataChunker");

    pelican::AbstractProtocol *protocol = new pelican::PelicanProtocol();
    server.addProtocol(protocol, 2000);

    server.start();

    while (!server.isReady()) {}

    return app.exec();
  }
  catch (const QString& error)
  {
    qFatal("%s", qPrintable(error));
  }
}
