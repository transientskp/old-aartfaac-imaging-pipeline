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
}

void CalibratorTest::stefCal()
{
}

void CalibratorTest::kronecker()
{
  MatrixXcf A(2,2), B(2,2), C(4,4);
  A << std::complex<float>(1.0f, 1.0f),
       std::complex<float>(2.0f, 2.0f),
       std::complex<float>(3.0f, 3.0f),
       std::complex<float>(4.0f, 4.0f);
  B = A.transpose();
  mCalibrator->kronecker(A, B, C);

  MatrixXf D(4,4);
  D <<  2.0f,  6.0f,  4.0f, 12.0f,
        4.0f,  8.0f,  8.0f, 16.0f,
        6.0f, 18.0f,  8.0f, 24.0f,
       12.0f, 24.0f, 16.0f, 32.0f;

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      CPPUNIT_ASSERT_EQUAL(C(i,j).imag(), D(i,j));
}

void CalibratorTest::khatrirao()
{
  MatrixXcf A(2,2), B(2,2), C(4,2);
  A << std::complex<float>(1.0f, 1.0f),
       std::complex<float>(2.0f, 2.0f),
       std::complex<float>(3.0f, 3.0f),
       std::complex<float>(4.0f, 4.0f);
  B = A.transpose();
  mCalibrator->khatrirao(A, B, C);

  MatrixXf D(4,2);
  D <<  2.0f, 12.0f,
        4.0f, 16.0f,
        6.0f, 24.0f,
       12.0f, 32.0f;

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 2; j++)
      CPPUNIT_ASSERT_EQUAL(C(i,j).imag(), D(i,j));
}
