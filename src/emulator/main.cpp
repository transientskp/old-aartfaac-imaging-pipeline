#include "stream/StreamEmulator.h"
#include "../utilities/Logger.h"
#include "version.h"

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/Config.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

void sighandler(int signal)
{
  qCritical("Received signal %d (%s), exit now", signal, strsignal(signal));
  Logger::close();
  exit(signal);
}

void usage(int signal)
{
  std::cout << "Usage: aartfaac-emulator <XML> [OPTION]" << std::endl;
  std::cout << " Emulates the gpu-based correlator using a casa ms." << std::endl;
  std::cout << " Configurable through xml file." << std::endl;
  std::cout << " Example: aartfaac-emulator emulator.xml O2" << std::endl << std::endl;

  std::cout << " XML\tlocation of the xml config file" << std::endl;
  std::cout << " OPTION\toption set in the xml config file" << std::endl;
  exit(signal);
}

int main(int argc, char *argv[])
{
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);

  Logger::open("aartfaac-emulator");
  qInstallMsgHandler(Logger::messageHandler);

  QCoreApplication app(argc, argv);
  QString xml_file;
  QString option;

  switch (argc)
  {
  case 2: xml_file = argv[1]; option = "O1"; break;
  case 3: xml_file = argv[1]; option = argv[2]; break;
  default: usage(EXIT_FAILURE);
  }

  try
  {
    pelican::Config config(xml_file);
    pelican::Config::TreeAddress address;
    address << pelican::Config::NodeId("configuration", "");
    address << pelican::Config::NodeId("StreamEmulator", option);
    pelican::ConfigNode settings;

    if (config.verifyAddress(address))
      settings = config.get(address);
    else
      throw QString("Invalid xml option set");

    pelican::EmulatorDriver driver(new StreamEmulator(settings));
    return app.exec();
  }
  catch (const QString &error)
  {
    qFatal("Error: %s", qPrintable(error));
  }

  return EXIT_SUCCESS;
}
