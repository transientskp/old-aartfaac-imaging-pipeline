#ifndef CALIBRATOR_MODULE_H
#define CALIBRATOR_MODULE_H

#include <pelican/core/AbstractModule.h>
#include <eigen3/Eigen/Dense>
#include <complex>
#include <vector>
#include <cmath>
#include "../../../Constants.h"
#include "../../../utilities/Utils.h"

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
               const double inFrequency,
                     MatrixXf &ioMask,
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
                      const MatrixXf  &inInvMask,
                            VectorXcf &outGains,
                            VectorXf  &outSourcePowers,
                            MatrixXcf &outNoiseCovMatrix);

  void wsfSrcPos(const MatrixXcf &inData,
                 const MatrixXcf &inSigma1,
                 const VectorXcf &inGains,
                 const double inFreq,
                       MatrixXd &ioPositions);

  class WSFCost
  {
  public:
    WSFCost(const MatrixXcf &inW, const MatrixXcf &inG, const double inFreq, const MatrixXd &inP, const int n);

    float operator()(const VectorXd &theta);

  private:
    const MatrixXcf &W;
    const MatrixXcf &G;
    const MatrixXd &P;
    const double freq;
    const int nsrc;

    MatrixXd src_pos;
    MatrixXcf T;
    MatrixXcf A;
    MatrixXcf Eye;
    MatrixXcf PAperp;
    std::complex<double> i1;
  };

  /// Initialized in the constructor and const
  MatrixXd mUVDist;
  VectorXd mRaSources;
  VectorXd mDecSources;
  VectorXi mEpoch;

  /// Changed when new antenna are (un)flagged
  std::vector<int> mFlagged;
  MatrixXf mSpatialFilterMask;
  MatrixXd mAntennaITRFReshaped;
  MatrixXf mMask;
  MatrixXd mSelection;
  MatrixXcf mNormalizedData;
  MatrixXcf mNoiseCovMatrix;

  VectorXcf mGains;
  VectorXf mFluxes;
  double mFrequency;
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
