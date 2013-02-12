#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"
#include "../../../utilities/Utils.h"

#include <pelican/utility/Config.h>
#include <QtCore>
#include <QDebug>

Calibrator::Calibrator(const ConfigNode &inConfig):
  AbstractModule(inConfig)
{
}

Calibrator::~Calibrator()
{
}

void Calibrator::run(const StreamBlob *input, StreamBlob *output)
{
  Q_UNUSED(input);
  Q_UNUSED(output);
}

int Calibrator::walsCalibration(const MatrixXcf &inModel,  					// A
                                const MatrixXcf &inData,   					// Rhat
                                const VectorXf  &inFluxes, 					// sigmas
                                      VectorXcf &outGains,          // g
                                      VectorXf  &outSourcePowers,   // sigmas
                                      MatrixXcf &outNoiseCovMatrix) // Sigma_n
{
  static const int max_iterations = 1;
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
    // 1. Per sensor gain estimation using gainSolv
    MatrixXcf model = inModel * prev_fluxes.asDiagonal() * inModel.adjoint(); // TODO: mask
    MatrixXcf data = inData; // TODO: mask
    gainSolv(model, data, prev_gains, cur_gains);

    MatrixXcf GA = cur_gains.asDiagonal() * inModel;
    MatrixXcf Rest = GA * prev_fluxes.asDiagonal() * GA.adjoint();
    Rest.resize(Rest.size(), 1);
    data.resize(1, Rest.size());
    MatrixXcf tmp(Rest);
    utils::pseudoInverse<std::complex<float> >(Rest, tmp);
    Rest = tmp.array() * data.array();
    float normg = std::abs(std::sqrt(Rest.sum()));
    cur_gains = normg * cur_gains / (cur_gains(0) / abs(cur_gains(0)));

    // 2. Model source flux estimation
    MatrixXcf inv_data = inData.inverse();
    GA = cur_gains.asDiagonal() * inModel;
    MatrixXf tmp0 = (GA.adjoint() * inv_data * GA).conjugate().array().abs();
    MatrixXf tmp1 = tmp0.cwiseProduct(tmp0).inverse();
    MatrixXcf tmp2 = (GA.adjoint() * inv_data * (inData - outNoiseCovMatrix) * inv_data * GA).diagonal();
    cur_fluxes = (tmp1 * tmp2).array().real(); // TODO: Check for infinity
    Q_ASSERT(abs(cur_fluxes(0)) > epsilon);
    cur_fluxes /= cur_fluxes(0);

    for (int j = 0; j < cur_fluxes.size(); j++)
      cur_fluxes(j) = std::max<float>(cur_fluxes(j), 0.0f);

    // 3. Noise covariance estimation

    // Test for convergence

    prev_gains = cur_gains;
    prev_fluxes = cur_fluxes;
  }

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

  static int n = inModel.rows();

  Q_ASSERT(n == inModel.rows());

  // allocate once
  static MatrixXcf data_normalised(n, n);
  static MatrixXcf data_calibrated(n, n);
  static VectorXcf estimated_calibration(n);
  static VectorXcf tmp(n);

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
