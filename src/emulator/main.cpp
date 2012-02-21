#include "UniboardEmulator.h"
#include "../utilities/Logger.h"
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
    QCoreApplication::exit(signal);
  }
};

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: aartfaac-emulator <table dir>" << std::endl;
    exit(EXIT_FAILURE);
  }

  CleanExit clean_exit;
  Logger::setLogFileProperties(NAME"-emulator.log", 10, 1024*1024*10);
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);
  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  pelican::ConfigNode xml_node(
        "<UniboardEmulator>"
        "  <packet samples=\"33\" />"
        "  <connection host=\"127.0.0.1\" port=\"2001\" />"
        "</UniboardEmulator>"
  );

  pelican::EmulatorDriver driver(new UniboardEmulator(xml_node));
  return app.exec();
}
