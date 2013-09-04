#include "UniboardEmulatorTest.h"
#include "../stream/StreamEmulator.h"

#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtNetwork/QTcpServer>

CPPUNIT_TEST_SUITE_REGISTRATION(UniboardEmulatorTest);

UniboardEmulatorTest::UniboardEmulatorTest()
  : CppUnit::TestFixture(),
    mApp(NULL)
{
}

UniboardEmulatorTest::~UniboardEmulatorTest()
{

}

void UniboardEmulatorTest::setUp()
{
  static int argc = 2;
  static char *argv[] = {(char *)"emulatortest", (char *)"../src/xml/emulatorConfig.xml"};
  mApp = new QCoreApplication(argc, argv);
  mServer = new QTcpServer();
}

void UniboardEmulatorTest::tearDown()
{
  mServer->close();
  delete mApp;
  delete mServer;
}

void UniboardEmulatorTest::emulate()
{
  pelican::ConfigNode xml_node(
    "<StreamEmulator>"
      "<connection     host=\"127.0.0.1\" port=\"4100\" />"
      "<measurementset name=\"/opt/aartfaac/TEST.MS\" />"
      "<emulator       packetInterval=\"0\"/>"
    "</StreamEmulator>"
  );

  pelican::EmulatorDriver driver(new StreamEmulator(xml_node));
  mServer->listen(QHostAddress("127.0.0.1"), 4100);
  mServer->waitForNewConnection(30000);
  QIODevice *connection = (QIODevice*) mServer->nextPendingConnection();
  CPPUNIT_ASSERT(mApp->exec() == 0);
  connection->waitForReadyRead(1e3);
  CPPUNIT_ASSERT_EQUAL(2664448ll, connection->bytesAvailable());
}
