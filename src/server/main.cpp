#include "../utilities/Logger.h"
#include "version.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QString>

int main(int argc, char* argv[])
{
  Logger::setLogFileProperties(NAME"-server.log", 10, 1024*1024*10);
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  return app.exec();
}
