#ifndef FLAGGER_TEST_H
#define FLAGGER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Flagger;
class StreamBlob;

class FlaggerTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(FlaggerTest);
  CPPUNIT_TEST(flag);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void flag();

  FlaggerTest();
  ~FlaggerTest() {}

private:
  Flagger *mFlagger;
  StreamBlob *mStreamBlob;
};

#endif // FLAGGER_TEST_H
