#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"
#include "../../../utilities/Utils.h"

#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/utility/Config.h>
#include <QtCore>
#include <QDebug>

extern char *gTableName;

Calibrator::Calibrator(const ConfigNode &inConfig):
  AbstractModule(inConfig)
{
  casa::MeasurementSet ms(gTableName);
  casa::ROMSColumns msc(ms);

  mAntennaITRF.resize(NUM_ANTENNAS, 3);
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    mAntennaITRF(a, 0) = msc.antenna().position()(a)(casa::IPosition(1,0));
    mAntennaITRF(a, 1) = msc.antenna().position()(a)(casa::IPosition(1,1));
    mAntennaITRF(a, 2) = msc.antenna().position()(a)(casa::IPosition(1,2));
  }

  mRaSources.resize(4);
  mDecSources.resize(4);

  // Cassiopeia A
  mRaSources(0)   = 6.1138f;
  mDecSources(0)  = 1.0219f;

  // Cygnus A
  mRaSources(1)   = 5.2262f;
  mDecSources(1)  = 0.7086f;

  // Tauras A
  mRaSources(2)   = 1.4464f;
  mDecSources(2)  = 0.3835f;

  // Virgo A
  mRaSources(3)   = 3.2651f;
  mDecSources(3)  = 0.2211f;

  mEpoch.resize(4);
  mEpoch.setZero();
  mEpoch.array() += 1;

  mNormalizedData.resize(NUM_ANTENNAS, NUM_ANTENNAS);
}

Calibrator::~Calibrator()
{
}

void Calibrator::run(const StreamBlob *input, StreamBlob *output)
{
  double time = input->mMJDTime / 86400.0 + 2400000.5;
  mNormalizedData = input->mXX;

  // Whitening of the array covariance matrix for DOA estimation
  float sqrt_dot = mNormalizedData.diagonal().norm();
  mNormalizedData.array() /= sqrt_dot;

  // Initial calibration
  statCal(mNormalizedData, time, input->mFrequency, mCalibrations, mSigmas, output->mXX);
}

void Calibrator::statCal(const MatrixXcf &inData,
                         const double inTime,
                         const double inFrequency,
                         VectorXcf &outCalibrations,
                         VectorXf &outSigmas,
                         MatrixXcf &outVisibilities)
{
  static const Vector3f normal(0.598753f, 0.072099f, 0.797682f);
  static const float lightspeed = 299792458.0f;

  MatrixXf src_pos(mRaSources.rows(), 3);
  utils::radec2itrf<float>(mRaSources, mDecSources, mEpoch, inTime, src_pos);

  //VectorXf up = src_pos * normal;
  std::complex<float> i1(0.0f, 1.0f);
  i1 *= 2.0f * M_PI * inFrequency / lightspeed;
  MatrixXcf A = (i1 * mAntennaITRF * src_pos.transpose()).array().exp();

  MatrixXcf KA(A.rows()*A.rows(), A.cols());
  utils::khatrirao<std::complex<float> >(A.conjugate(), A, KA);

  MatrixXf AA = (A.adjoint() * A).array().abs().square();
  MatrixXf AAi(AA.rows(), AA.cols());
  utils::pseudoInverse<float>(AA, AAi);

  MatrixXcf data(inData);
  data.resize(inData.rows()*inData.cols(), 1);
  VectorXf flux = (AAi * KA.adjoint() * data).array().real();
  flux.array() /= flux(0);

  walsCalibration(A, inData, flux, outCalibrations, outSigmas, outVisibilities);
  outCalibrations.array() /= 1.0f;
  outCalibrations.adjointInPlace();
}


int Calibrator::walsCalibration(const MatrixXcf &inModel,  					// A
                                const MatrixXcf &inData,   					// Rhat
                                const VectorXf  &inFluxes, 					// sigmas
                                      VectorXcf &outGains,          // g
                                      VectorXf  &outSourcePowers,   // sigmas
                                      MatrixXcf &outNoiseCovMatrix) // Sigma_n
{
  static const int max_iterations = 10;
  static const float epsilon = 1e-10f;

  int i;
  Q_ASSERT(outNoiseCovMatrix.rows() == inData.rows());
  Q_ASSERT(outNoiseCovMatrix.cols() == inData.cols());

  outNoiseCovMatrix.setZero();
  VectorXcf cur_gains(inData.rows());
  VectorXcf prev_gains(inData.rows());
  for (int i = 0; i < inData.rows(); i++)
    prev_gains(i) = std::complex<float>(1.0f, 1.0f);
  VectorXf cur_fluxes(inFluxes.rows());
  VectorXf prev_fluxes(inFluxes);

  for (i = 1; i <= max_iterations; i++)
  {
    // ======================================================
    // ==== 1. Per sensor gain estimation using gainSolv ====
    // ======================================================
    MatrixXcf model = inModel * prev_fluxes.asDiagonal() * inModel.adjoint(); // TODO: mask
    MatrixXcf data = inData; // TODO: mask
    gainSolv(model, data, prev_gains, cur_gains);

    MatrixXcf GA = cur_gains.asDiagonal() * inModel;
    MatrixXcf Rest = GA * prev_fluxes.asDiagonal() * GA.adjoint();

    Rest.resize(Rest.size(), 1);
    data.resize(Rest.size(), 1);

    MatrixXcf X(Rest.rows(), Rest.cols());
    utils::pseudoInverse<std::complex<float> >(Rest, X);
    float normg = std::abs(std::sqrt((X.array() * data.array()).sum()));
    cur_gains = normg * cur_gains / (cur_gains(0) / abs(cur_gains(0)));

    // =========================================
    // ==== 2. Model source flux estimation ====
    // =========================================
    MatrixXcf inv_data = inData.inverse();
    GA = cur_gains.asDiagonal() * inModel;
    MatrixXf tmp0 = (GA.adjoint() * inv_data * GA).conjugate().array().abs();
    MatrixXf tmp1 = tmp0.cwiseProduct(tmp0).inverse();
    MatrixXcf tmp2 = (GA.adjoint() * inv_data * (inData - outNoiseCovMatrix) * inv_data * GA).diagonal();
    cur_fluxes = (tmp1 * tmp2).array().real(); // TODO: Check for infinity
    cur_fluxes /= cur_fluxes(0); // TODO: Make sure cur_fluxes(0) != 0

    for (int j = 0; j < cur_fluxes.size(); j++)
      cur_fluxes(j) = std::max<float>(cur_fluxes(j), 0.0f);

    // ========================================
    // ==== 3. Noise covariance estimation ====
    // ========================================
    outNoiseCovMatrix = (inData - GA * cur_fluxes.asDiagonal() * GA.adjoint()); // TODO: mask

    // =================================
    // ==== 4. Test for convergence ====
    // =================================
    MatrixXcf prev_theta(prev_gains.size() + prev_fluxes.size(), 1);
    MatrixXcf cur_theta(cur_gains.size() + cur_fluxes.size(), 1);
    for (int j = 0; j < prev_gains.size(); j++)
    {
      prev_theta(j) = prev_gains(j);
      cur_theta(j) = cur_gains(j);
    }
    for (int j = 0; j < prev_fluxes.size(); j++)
    {
      prev_theta(j+prev_gains.size()) = std::complex<float>(prev_fluxes(j), 0.0f);
      cur_theta(j+prev_gains.size()) = std::complex<float>(cur_fluxes(j), 0.0f);
    }

    MatrixXcf tmp(prev_theta.rows(), prev_theta.cols());
    utils::pseudoInverse<std::complex<float> >(prev_theta, tmp);
    std::complex<float> x = (tmp.array() * cur_theta.array()).sum();
    float sum = abs(x - 1.0f);
    if (sum < epsilon)
    {
      qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
      break;
    }

    // ================================
    // ==== Prepare next iteration ====
    // ================================
    prev_gains = cur_gains;
    prev_fluxes = cur_fluxes;
  }

  outGains = cur_gains;
  outSourcePowers = cur_fluxes;

  if (i >= max_iterations)
    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);

  return i;
}

int Calibrator::gainSolv(const MatrixXcf &inModel,
                         const MatrixXcf &inData,
                         const VectorXcf &inEstimatedGains,
                               VectorXcf &outGains)
{
  Q_ASSERT(inModel.rows() == inModel.cols());
  Q_ASSERT(inData.rows() == inData.cols());
  Q_ASSERT(inModel.rows() == inData.rows());
  Q_ASSERT(inModel.rows() == inEstimatedGains.rows());
  Q_ASSERT(inEstimatedGains.rows() == outGains.rows());

  int n = inModel.rows();

  // allocate once
  MatrixXcf data_normalised(n, n);
  MatrixXcf data_calibrated(n, n);
  VectorXcf estimated_calibration(n);
  VectorXcf tmp(n);

  static const int max_iterations = 100;
  static const float epsilon = 1e-6f;

  for (int i = 0; i < n; i++)
  {
    // Normalize the data
    data_normalised.col(i) = inData.col(i) / inData.col(i).dot(inData.col(i));

    // Initial calibration
    data_calibrated.col(i) = inEstimatedGains.array() * inModel.col(i).array();
  }

  int i;
  for (i = 1; i <= max_iterations; i++)
  {
    for (int j = 0; j < n; j++)
    {
      estimated_calibration(j) = data_normalised.col(j).dot(data_calibrated.col(j));
      outGains(j) = 1.0f / std::conj(estimated_calibration(j));
    }

    if (i % 2 == 1)
      tmp = outGains;
    else
    {
      // Update gains and check for convergence
      estimated_calibration = outGains;
      outGains = (outGains.array() + tmp.array()) / 2.0f;
      float gains_normal = outGains.norm();
      tmp = outGains.array() - estimated_calibration.array();
      float delta_gains_normal = tmp.norm();
      if (delta_gains_normal / gains_normal <= epsilon)
      {
        qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
        break;
      }
    }

    // Update calibration of array covariance matrix
    for (int j = 0; j < n; j++)
      data_calibrated.col(j) = outGains.array() * inModel.col(j).array();
  }

  if (i >= max_iterations)
    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);

  return i;
}
