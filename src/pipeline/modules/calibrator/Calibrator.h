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
  void statCal(const MatrixXcd &inData,
               const double inFrequency,
                     MatrixXd &ioMask,
                     VectorXcd &outCalibrations,
                     VectorXd &outSigmas,
                     MatrixXcd &outVisibilities);

  int gainSolv(const MatrixXcd &inModel,
               const MatrixXcd &inData,
               const VectorXcd &inEstimatedGains,
                     VectorXcd &outGains);

  int walsCalibration(const MatrixXcd &inModel,
                      const MatrixXcd &inData,
                      const VectorXd  &inFluxes,
                      const MatrixXd  &inInvMask,
                            VectorXcd &outGains,
                            VectorXd  &outSourcePowers,
                            MatrixXcd &outNoiseCovMatrix);

  void wsfSrcPos(const MatrixXcd &inData,
                 const MatrixXcd &inSigma1,
                 const VectorXcd &inGains,
                 const double inFreq,
                       MatrixXd &ioPositions);

  class WSFCost
  {
  public:
    WSFCost(const MatrixXcd &inW, const MatrixXcd &inG, const double inFreq, const MatrixXd &inP):
      W(inW),
      G(inG),
      freq(inFreq),
      P(inP)
    {
    }

    double operator()(const VectorXd &theta);

  private:
    const MatrixXcd &W;
    const MatrixXcd &G;
    const double freq;
    const MatrixXd &P;
  };

  /// Initialized in the constructor and const
  MatrixXd mUVDist;
  VectorXd mRaSources;
  VectorXd mDecSources;
  VectorXi mEpoch;

  /// Changed when new antenna are (un)flagged
  std::vector<int> mFlagged;
  MatrixXd mSpatialFilterMask;
  MatrixXd mAntennaITRFReshaped;
  MatrixXd mMask;
  MatrixXd mSelection;
  MatrixXcd mNormalizedData;
  MatrixXcd mNoiseCovMatrix;

  VectorXcd mGains;
  VectorXd mFluxes;
  double mFrequency;
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
