#include "CalibratorTest.h"
#include "../modules/calibrator/Calibrator.h"
#include "../StreamBlob.h"
#include "../../Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(CalibratorTest);


CalibratorTest::CalibratorTest():
  CppUnit::TestFixture(),
  mCalibrator(NULL)
{
}

void CalibratorTest::setUp()
{
  pelican::ConfigNode config(
    "<Calibrator>"
    "  <output path=\"/dev/null\"/>"
    "  <uvw path=\"/opt/aartfaac/uvw_merged.txt\"/>"
    "  <lba type=\"1\"/> <!-- 0 = LBA_INNER, 1 = LBA_OUTER -->"
    "</Calibrator>"
  );
  mCalibrator = new Calibrator(config);
}

void CalibratorTest::tearDown()
{
  delete mCalibrator;
}

void CalibratorTest::gainSolve()
{
  MatrixXcf model(2, 2);
  model << std::complex<float>(1.0f, 5.0f), std::complex<float>(2.0f, 6.0f),
           std::complex<float>(3.0f, 7.0f), std::complex<float>(4.0f, 8.0f);
  MatrixXcf data(model);
  VectorXcf initial_gains(2);
  initial_gains << std::complex<float>(1.0f, 0.0f), std::complex<float>(1.0f, 0.0f);

  VectorXcf gains(2);
  int iters = mCalibrator->gainSolv(model, data, initial_gains, gains);

  data = model.array() * (gains * gains.transpose()).array();

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(model(i,j).real(), data(i,j).real(), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(model(i,j).imag(), data(i,j).imag(), 1e-6);
    }
  }

  CPPUNIT_ASSERT_EQUAL(2, iters);
}

void CalibratorTest::stefCal()
{
}
