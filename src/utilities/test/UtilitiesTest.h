#ifndef UTILITIES_TEST_H
#define UTILITIES_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <eigen3/Eigen/Dense>

using namespace Eigen;

class UtilitiesTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(UtilitiesTest);
  CPPUNIT_TEST(dates);
  CPPUNIT_TEST(kronecker);
  CPPUNIT_TEST(khatrirao);
  CPPUNIT_TEST(polyval);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void dates();
  void kronecker();
  void khatrirao();
  void polyval();

  UtilitiesTest();
  ~UtilitiesTest() {}

private:
};

#endif // UTILITIES_TEST_H

