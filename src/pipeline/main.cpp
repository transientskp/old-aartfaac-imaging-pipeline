#include "../utilities/Logger.h"
#include "version.h"
#include "UniboardPipeline.h"
#include "StreamAdapter.h"
#include "ServiceAdapter.h"

#include <pelican/core/PipelineApplication.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

int main(int argc, char *argv[])
{
  Logger::open(NAME"-pipeline");
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");
  qDebug("%s", HUMAN_NAME);

  try
  {
    pelican::PipelineApplication p_app(argc, argv);
    p_app.registerPipeline(new UniboardPipeline());
    p_app.setDataClient("PelicanServerClient");
    p_app.start();
  }
  catch (const QString &error)
  {
    qFatal("%s", qPrintable(error));
  }

  return app.exec();
}
