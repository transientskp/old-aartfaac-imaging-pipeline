#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"

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
    // Bootstrap with estimated gains
    outGains(i) = inEstimatedGains(i);

    // Normalize the data
    data_normalised.col(i) = inData.col(i).array() / inData.col(i).adjoint().dot(inData.col(i));

    // Initial calibration
    data_calibrated.col(i) = inEstimatedGains.array() * inModel.col(i).array();
  }

  int i;
  for (i = 1; i <= max_iterations; i++)
  {
    for (int j = 0; j < n; j++)
    {
      estimated_calibration(j) = data_normalised.col(j).adjoint().dot(data_calibrated.col(j));
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
