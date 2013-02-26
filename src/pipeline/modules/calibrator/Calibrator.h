#ifndef CALIBRATOR_MODULE_H
#define CALIBRATOR_MODULE_H

#include <pelican/core/AbstractModule.h>
#include <eigen3/Eigen/Dense>
#include <complex>

using namespace pelican;
using namespace Eigen;

class StreamBlob;

class Calibrator : public AbstractModule
{
friend class CalibratorTest;

public:
  Calibrator(const ConfigNode &inConfig);
  ~Calibrator();

  void run(const StreamBlob *input, StreamBlob *output);

private:
  void statCal(const MatrixXcf &inData,
               const double inTime,
               const double inFrequency,
               VectorXcf &outCalibrations,
               VectorXf &outSigmas,
               MatrixXcf &outVisibilities);

  int gainSolv(const MatrixXcf &inModel,
               const MatrixXcf &inData,
               const VectorXcf &inEstimatedGains,
                     VectorXcf &outGains);

  int walsCalibration(const MatrixXcf &inModel,
                      const MatrixXcf &inData,
                      const VectorXf  &inFluxes,
                            VectorXcf &outGains,
                            VectorXf  &outSourcePowers,
                            MatrixXcf &outNoiseCovMatrix);

  MatrixXf mAntennaITRF;
  MatrixXcf mNormalizedData;

  VectorXcf mCalibrations;
  VectorXf mSigmas;

  VectorXf mRaSources;
  VectorXf mDecSources;
  VectorXi mEpoch;
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
