#include <QtCore>

#include "MetaData.h"
#include "version.h"
#include "../utilities/Logger.h"

int main(int argc, char *argv[])
{
  Logger::open(NAME"-metadata");
  qInstallMsgHandler(Logger::messageHandler);
  QCoreApplication app(argc, argv);

  if (argc != 2)
  {
    std::cerr << "Usage: aartfaac-metadata <MeasurementSet>" << std::endl;
    exit(EXIT_FAILURE);
  }

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  MetaData meta_data;
  meta_data.start();

  return 0;
}
