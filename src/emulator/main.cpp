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
  std::cout << "Usage: aartfaac-emulator <XML>" << std::endl;
  std::cout << " Emulates the gpu-based correlator using a casa ms." << std::endl;
  std::cout << " Configurable through xml file." << std::endl;
  std::cout << " Example: aartfaac-emulator emulator.xml" << std::endl << std::endl;

  std::cout << " XML\tlocation of the xml config file" << std::endl;
  exit(signal);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    usage(EXIT_FAILURE);  signal(SIGTERM, &sighandler);

  signal(SIGINT, &sighandler);

  Logger::open("aartfaac-emulator");
  qInstallMsgHandler(Logger::messageHandler);

  QString xml_file(argv[1]);
  QCoreApplication app(argc, argv);

  try
  {
    pelican::Config config(xml_file);
    pelican::Config::TreeAddress address;
    address << pelican::Config::NodeId("configuration", "");
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
