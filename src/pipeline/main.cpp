#include "../utilities/Logger.h"
#include "version.h"
#include "UniboardPipeline.h"
#include "StreamAdapter.h"
#include "ServiceAdapter.h"

#include <csignal>
#include <pelican/core/PipelineApplication.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

char *gTableName; ///< Used in the Visibilities class for constructing an ms

void sighandler(int signal)
{
  qCritical("Received signal %d (%s), exit now", signal, strsignal(signal));
  Logger::close();
  exit(signal);
}

int main(int argc, char *argv[])
{
  Logger::open("aartfaac-pipeline");
  qInstallMsgHandler(Logger::messageHandler);

  if (argc != 3)
    qFatal("Wrong number of arguments");

  gTableName = argv[2];

  // HACK: As the pipeline app doesn't allow more args on the commandline, we
  //       strip them off again here.
  int   ac    = argc - 1;
  char *av[]  = {argv[0], argv[1]};

  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  signal(SIGQUIT, &sighandler);

  QCoreApplication app(ac, av);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");
  std::cout << HUMAN_NAME << std::endl;

  try
  {
    pelican::PipelineApplication p_app(ac, av);
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
