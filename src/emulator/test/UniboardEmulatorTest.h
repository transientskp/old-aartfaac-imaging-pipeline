#include <cppunit/extensions/HelperMacros.h>

class QCoreApplication;

class UniboardEmulatorTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(UniboardEmulatorTest);
  CPPUNIT_TEST(emulate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void emulate();

  UniboardEmulatorTest();
  ~UniboardEmulatorTest();

private:
  QCoreApplication *mApp;
};
