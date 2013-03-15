#ifndef CALIBRATOR_TEST_H
#define CALIBRATOR_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Calibrator;
class StreamBlob;

class CalibratorTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(CalibratorTest);
  CPPUNIT_TEST(gainSolve);
  CPPUNIT_TEST(walsCalibration);
  //CPPUNIT_TEST(calibrate);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void gainSolve();
  void walsCalibration();
  void calibrate();

  CalibratorTest();
  ~CalibratorTest() {}

private:
  Calibrator *mCalibrator;
  StreamBlob *mStreamBlob;
};

#endif // CALIBRATOR_TEST_H

