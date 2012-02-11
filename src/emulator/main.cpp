#include "UniboardEmulator.h"

#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  pelican::ConfigNode xmlNode(
        "<UniboardEmulator>"
        "  <packet samples=\"256\" />"
        "  <connection host=\"127.0.0.1\" port=\"2001\" />"
        "</UniboardEmulator>"
  );

  pelican::EmulatorDriver driver(new UniboardEmulator(xmlNode));
  return app.exec();
}
