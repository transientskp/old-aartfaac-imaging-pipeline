#include "../utilities/Logger.h"
#include "version.h"
#include "UniboardPipeline.h"
#include "StreamAdapter.h"

#include <pelican/core/PipelineApplication.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

void usage(int signal)
{
  std::cout << "Usage: aartfaac-pipeline <XML>" << std::endl;
  std::cout << " Creates an aartfaac pipeline for processing." << std::endl;
  std::cout << " Configurable through xml file." << std::endl;
  std::cout << " Example: aartfaac-pipeline pipeline.xml" << std::endl << std::endl;

  std::cout << " XML\tlocation of the xml config file" << std::endl;
  exit(signal);
}


int main(int argc, char *argv[])
{
  if (argc != 2)
    usage(EXIT_FAILURE);

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
    int threads = node.getOption("pipeline", "threads", "1").toInt();
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
