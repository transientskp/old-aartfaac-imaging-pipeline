#include "UniboardEmulatorTest.h"
#include "../UniboardEmulator.h"

#include <pelican/emulator/EmulatorDriver.h>
#include <pelican/utility/ConfigNode.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>

CPPUNIT_TEST_SUITE_REGISTRATION(UniboardEmulatorTest);

UniboardEmulatorTest::UniboardEmulatorTest()
  : CppUnit::TestFixture()
{

}

UniboardEmulatorTest::~UniboardEmulatorTest()
{

}

void UniboardEmulatorTest::setUp()
{
  int argc = 2;
  char *argv[] = {(char*)"emulatortest", (char*)"/opt/aartfaac/testset"};
  mApp = new QCoreApplication(argc, argv);
}

void UniboardEmulatorTest::tearDown()
{
  delete mApp;
}

void UniboardEmulatorTest::emulate()
{
  pelican::ConfigNode xml_node(
        "<UniboardEmulator>"
        "  <connection host=\"127.0.0.1\" port=\"2001\" />"
        "</UniboardEmulator>"
  );

  pelican::EmulatorDriver driver(new UniboardEmulator(xml_node));
	mApp->exec();
}
