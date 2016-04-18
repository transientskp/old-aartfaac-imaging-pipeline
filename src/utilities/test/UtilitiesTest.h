#ifndef UTILITIES_TEST_H
#define UTILITIES_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <eigen3/Eigen/Dense>

using namespace Eigen;

class UtilitiesTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(UtilitiesTest);
  CPPUNIT_TEST(kronecker);
  CPPUNIT_TEST(khatrirao);
  CPPUNIT_TEST(polyval);
  CPPUNIT_TEST(sph2cart);
  CPPUNIT_TEST(precession);
  CPPUNIT_TEST(radec2itrf);
  CPPUNIT_TEST(pseudoinv);
  CPPUNIT_TEST(simplex);
  CPPUNIT_TEST(timeconversions);
  CPPUNIT_TEST(subband2frequency);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void kronecker();
  void khatrirao();
  void polyval();
  void sph2cart();
  void precession();
  void radec2itrf();
  void pseudoinv();
  void simplex();
  void subband2frequency();
  void timeconversions();

  UtilitiesTest();
  ~UtilitiesTest() {}

private:
};

#endif // UTILITIES_TEST_H

