#include "../utilities/Logger.h"
#include "../utilities/monitoring/Server.h"
#include "version.h"
#include "UniboardPipeline.h"
#include "StreamAdapter.h"

#include <pelican/core/PipelineApplication.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

int main(int argc, char *argv[])
{
  Logger::open("aartfaac-pipeline");
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");
  std::cout << HUMAN_NAME << std::endl;

  try
  {
    pelican::PipelineApplication p_app(argc, argv);
    pelican::Config::TreeAddress address;
    address << pelican::Config::NodeId("configuration", "");
    pelican::ConfigNode node = p_app.config()->get(address);
    int port = node.getOption("pipeline", "monport").toInt();
    int threads = node.getOption("pipeline", "threads", "1").toInt();
    Server::CreateInstance(port);
    p_app.registerPipeline(new UniboardPipeline(threads));
    p_app.setDataClient("PelicanServerClient");
    p_app.start();
  }
  catch (const QString &error)
  {
    qFatal("%s", qPrintable(error));
  }

  Logger::close();

  return EXIT_SUCCESS;
}
