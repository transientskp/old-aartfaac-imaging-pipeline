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
  {
    model(i,i) = std::complex<float>(0.0f, 0.0f);
    for (int j = 0; j < n; j++)
      model(i,j).imag() = -model(i,j).imag();
  }

  VectorXcf gains(antennas);
  gains += VectorXcf::Random(n) * 2.0f;
  MatrixXcf data = gains.asDiagonal() * model * gains.asDiagonal().toDenseMatrix().adjoint();

  VectorXcf recov(n), initial_gains(n);
  for (int i = 0; i < n; i++)
    initial_gains(i) = std::complex<float>(1.0f, 1.0f);

  mCalibrator->gainSolv(model, data, initial_gains, recov);

  float org = ((gains.adjoint() * gains).array().abs()(0));
  float rec = ((recov.adjoint() * recov).array().abs()(0));

  CPPUNIT_ASSERT_DOUBLES_EQUAL(org, rec, 1e-3f);
}

void CalibratorTest::walsCalibration()
{
  const int n = 3;
  VectorXcf antennas(n);
  antennas << std::complex<float>(10.0f, 20.0f),
              std::complex<float>(14.0f, 25.0f),
              std::complex<float>(12.0f, 22.0f);
  MatrixXcf model = antennas * antennas.adjoint();
  for (int i = 0; i < n; i++)
  {
    model(i,i) = std::complex<float>(0.0f, 0.0f);
    for (int j = 0; j < n; j++)
      model(i,j).imag() = -model(i,j).imag();
  }

  VectorXcf gains(n);
  gains << std::complex<float>(10.0f, 15.0f),
           std::complex<float>(24.0f, 10.0f),
           std::complex<float>(42.0f, 12.0f);
  MatrixXcf data = gains.asDiagonal() * model * gains.asDiagonal().toDenseMatrix().adjoint();

  VectorXf fluxes(n);
  for (int i = 0; i < n; i++)
    fluxes(i) = 1.0f;

  VectorXcf est_gains(n);
  VectorXf source_powers(n);
  MatrixXcf cov_matrix(n,n);

  mCalibrator->walsCalibration(model, data, fluxes, est_gains, source_powers, cov_matrix);

  //std::cout << "MODEL:\n" << model << std::endl;
}
