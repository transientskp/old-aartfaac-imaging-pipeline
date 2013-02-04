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
  const int n = 3;
  VectorXcf antennas = VectorXcf::Random(n) * 10.0f;
  MatrixXcf model = antennas * antennas.adjoint();
  for (int i = 0; i < n; i++)
    model(i,i) = std::complex<float>(0.0f, 0.0f);

  VectorXcf gains(antennas);
  gains += VectorXcf::Random(n);
  MatrixXcf data = gains.asDiagonal().toDenseMatrix().adjoint() * model * gains.asDiagonal();

  VectorXcf recov(n), initial_gains(n);
  for (int i = 0; i < n; i++)
    initial_gains(i) = std::complex<float>(1.0f, 1.0f);

  mCalibrator->gainSolv(model, data, initial_gains, recov);

  MatrixXf org = (gains * gains.transpose()).array().abs();
  MatrixXf rec = (recov * recov.transpose()).array().abs();

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(org(i,j), rec(i,j), 1e-2);
}

void CalibratorTest::walsCalibration()
{
  const int n = 3;
  VectorXcf antennas = VectorXcf::Random(n) * 10.0f;
  MatrixXcf model = antennas * antennas.adjoint();
  for (int i = 0; i < n; i++)
    model(i,i) = std::complex<float>(0.0f, 0.0f);

  VectorXcf gains(antennas);
  gains += VectorXcf::Random(n);
  MatrixXcf data = gains.asDiagonal().toDenseMatrix().adjoint() * model * gains.asDiagonal();

  VectorXcf initial_gains(n);
  for (int i = 0; i < n; i++)
    initial_gains(i) = std::complex<float>(1.0f, 1.0f);

  VectorXf est_gains(n), source_powers(n);
  MatrixXcf cov_matrix(n,n);

  mCalibrator->walsCalibration(model, data, initial_gains, est_gains, source_powers, cov_matrix);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, source_powers(0), 1e-3f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.304, source_powers(1), 1e-3f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, source_powers(2), 1e-3f);
}
