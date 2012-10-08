#include "ServiceEmulator.h"
#include "../../utilities/Logger.h"
#include "version.h"

#include <csignal>
#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

struct CleanExit {
  CleanExit()
  {
    signal(SIGINT, &CleanExit::exitQt);
    signal(SIGTERM, &CleanExit::exitQt);
    signal(SIGKILL, &CleanExit::exitQt);
  }

  static void exitQt(int signal)
  {
    Logger::close();
    QCoreApplication::exit(signal);
  }
};

int main(int argc, char* argv[])
{
  QString host = "127.0.0.1";
  QString port = "2002";

  CleanExit clean_exit;
  Logger::open(NAME"-service-emulator");
  qInstallMsgHandler(Logger::messageHandler);
  QCoreApplication app(argc, argv);

  if (argc != 2 && argc != 4)
  {
    std::cerr << "Usage: aartfaac-emulator <table dir> [[host] [port]]" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (argc == 4)
  {
    host = QCoreApplication::arguments().at(2);
    port = QCoreApplication::arguments().at(3);
  }

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");
  qDebug("%s", HUMAN_NAME);
  pelican::ConfigNode xml_node(
        "<UniboardEmulator>"
        "  <connection host=\"" + host + "\" port=\"" + port + "\" />"
        "</UniboardEmulator>"
  );

  pelican::EmulatorDriver driver(new ServiceEmulator(xml_node));
  return app.exec();
}
