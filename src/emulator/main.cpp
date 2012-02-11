#include "UniboardEmulator.h"
#include "version.h"

#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  app.setApplicationName(HUMAN_NAME);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName("Anton Pannekoek Institute");
  app.setOrganizationDomain("http://www.aartfaac.org");

  if (argc != 2)
  {
    qFatal("Usage: %s <table dir>", argv[0]);
  }

  qDebug("%s", HUMAN_NAME);

  pelican::ConfigNode xmlNode(
        "<UniboardEmulator>"
        "  <packet samples=\"12\" />"
        "  <connection host=\"127.0.0.1\" port=\"2001\" />"
        "</UniboardEmulator>"
  );

  pelican::EmulatorDriver driver(new UniboardEmulator(xmlNode));
  return app.exec();
}
