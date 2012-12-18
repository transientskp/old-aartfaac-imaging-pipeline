#include "stream/StreamEmulator.h"
#include "service/ServiceEmulator.h"
#include "../utilities/Logger.h"
#include "version.h"

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

void sighandler(int signal)
{
  qCritical("Received signal %d (%s), exit now", signal, strsignal(signal));
  Logger::close();
  exit(signal);
}

int main(int argc, char *argv[])
{
  QString host = "127.0.0.1";

  Logger::open("aartfaac-emulator");
  qInstallMsgHandler(Logger::messageHandler);

  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  signal(SIGQUIT, &sighandler);

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
  std::cout << HUMAN_NAME << std::endl;

  pelican::ConfigNode service_config(
    "<ServiceEmulator>"
    "  <connection host=\"" + host + "\" port=\"2002\" />"
    "</ServiceEmulator>"
  );
  pelican::EmulatorDriver driver1(new ServiceEmulator(service_config));

  sleep(1);

  pelican::ConfigNode stream_config(
    "<StreamEmulator>"
    "  <connection host=\"" + host + "\" port=\"2001\" />"
    "</StreamEmulator>"
  );
  pelican::EmulatorDriver driver2(new StreamEmulator(stream_config));

  return app.exec();
}
