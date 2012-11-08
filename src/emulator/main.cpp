#include "stream/StreamEmulator.h"
#include "service/ServiceEmulator.h"
#include "../utilities/Logger.h"
#include "version.h"

#include <iostream>
#include <csignal>
#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

struct CleanExit
{
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

int main(int argc, char *argv[])
{
  QString host = "127.0.0.1";

  CleanExit clean_exit;
  Logger::open(NAME"-emulator");
  qInstallMsgHandler(Logger::messageHandler);
  QCoreApplication app(argc, argv);

  if (argc != 2 && argc != 3)
  {
    std::cerr << "Usage: aartfaac-emulator <ms> [host]" << std::endl;
    std::cerr << "       ms  : location to measurement set" << std::endl;
    std::cerr << "       host: host address (pelican server)" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (argc == 3)
  {
    host = QCoreApplication::arguments().at(2);
  }

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");
  qDebug("%s", HUMAN_NAME);

  pelican::ConfigNode service_config(
    "<ServiceEmulator>"
    "  <connection host=\"" + host + "\" port=\"2002\" />"
    "</ServiceEmulator>"
  );
  pelican::EmulatorDriver driver1(new ServiceEmulator(service_config));

  pelican::ConfigNode stream_config(
    "<StreamEmulator>"
    "  <connection host=\"" + host + "\" port=\"2001\" />"
    "</StreamEmulator>"
  );
  pelican::EmulatorDriver driver2(new StreamEmulator(stream_config));

  return app.exec();
}
