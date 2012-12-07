#ifndef IMAGER_TEST_H
#define IMAGER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Imager;

class ImagerTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(ImagerTest);
  CPPUNIT_TEST(fftShift);
  CPPUNIT_TEST(gridding);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void fftShift();
  void gridding();

  ImagerTest();
  ~ImagerTest() {}

private:
  Imager *mImager;
};

#endif // IMAGER_TEST_H
