#ifndef CALIBRATOR_TEST_H
#define CALIBRATOR_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Calibrator;

class CalibratorTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(CalibratorTest);
  CPPUNIT_TEST(gainSolve);
  CPPUNIT_TEST(stefCal);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void gainSolve();
  void stefCal();

  CalibratorTest();
  ~CalibratorTest() {}

private:
  Calibrator *mCalibrator;
};

#endif // CALIBRATOR_TEST_H

