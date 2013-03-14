#ifndef CALIBRATOR_MODULE_H
#define CALIBRATOR_MODULE_H

#include <pelican/core/AbstractModule.h>
#include <eigen3/Eigen/Dense>
#include <complex>
#include <vector>
#include <cmath>
#include "../../../Constants.h"

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
                       MatrixXf &ioPositions);

  class WSFCost
  {
  public:
    WSFCost(const MatrixXcf &inW, const MatrixXcf &inG, const double inFreq, const MatrixXf &inP):
      W(inW),
      G(inG),
      freq(inFreq),
      P(inP)
    {
    }

    float operator()(vector<float> theta)
    {
      const int nsrc = theta.size() / 2;
      const int nelem = P.rows();

      MatrixXf src_pos(nsrc, 3);
      for (int i = 0; i < nsrc; i++)
      {
        src_pos(i, 0) = cosf(theta[i]) * cosf(theta[i+nsrc]);
        src_pos(i, 1) = sinf(theta[i]) * cosf(theta[i+nsrc]);
        src_pos(i, 2) = sinf(theta[i]+nsrc);
      }

      std::complex<float> i1(0.0f, 1.0f);
      i1 *= 2.0f * M_PI * freq / C_MS;
      MatrixXcf T = (-i1 * (P * src_pos.transpose())).array().exp();
      MatrixXcf A = G * T;
      MatrixXcf eye(nelem, nelem); eye.setIdentity();
      MatrixXcf PAperp = eye.array() - (A * (A.adjoint() * A).inverse() * A.adjoint()).array();
      return (PAperp * W).trace().real();
    }

  private:
    const MatrixXcf &W;
    const MatrixXcf &G;
    const double freq;
    const MatrixXf &P;
  };

  /// Initialized in the constructor and const
  MatrixXf mAntennaITRF;
  MatrixXf mUCoords;
  MatrixXf mVCoords;
  MatrixXf mUVDist;
  VectorXd mRaSources;
  VectorXd mDecSources;
  VectorXi mEpoch;

  /// Changed when new antenna are (un)flagged
  std::vector<int> mFlagged;
  MatrixXf mSpatialFilterMask;
  MatrixXf mAntennaITRFReshaped;
  MatrixXf mMask;
  MatrixXf mSelection;
  MatrixXcf mNormalizedData;
  MatrixXcf mNoiseCovMatrix;

  VectorXcf mGains;
  VectorXf mFluxes;
  double mFrequency;
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
