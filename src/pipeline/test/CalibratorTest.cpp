#include "CalibratorTest.h"
#include "../modules/calibrator/Calibrator.h"
#include "../StreamBlob.h"
#include "Constants.h"
#include "../../Macros.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(CalibratorTest);


CalibratorTest::CalibratorTest():
  CppUnit::TestFixture(),
  mCalibrator(NULL),
  mStreamBlob(NULL)
{
}

void CalibratorTest::setUp()
{
  pelican::ConfigNode config(
    "<Calibrator>"
      "<positrf path=\"/usr/local/share/aartfaac/antennasets/lba_outer.dat\" />"
    "</Calibrator>"
  );
  mCalibrator = new Calibrator(config);
  mStreamBlob = new StreamBlob();
  Data(mStreamBlob) = MatrixXcf::Random(NUM_ANTENNAS, NUM_ANTENNAS);
  Data(mStreamBlob).array() + Data(mStreamBlob).transpose().array();
  mStreamBlob->mHeader.time = 4.83793e+09;
  mStreamBlob->mHeader.subband = 296;
}

void CalibratorTest::tearDown()
{
  delete mCalibrator;
  delete mStreamBlob;
}

void CalibratorTest::calibrate()
{
  mCalibrator->run(0, mStreamBlob, mStreamBlob);
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

  CPPUNIT_ASSERT_DOUBLES_EQUAL(org, rec, 0.3);
}

void CalibratorTest::walsCalibration()
{
//  mCalibrator->walsCalibration(model, data, fluxes, mask, gains, powers, noisecov);
}
