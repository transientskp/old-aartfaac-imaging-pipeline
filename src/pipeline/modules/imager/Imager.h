#ifndef IMAGER_MODULE_H
#define IMAGER_MODULE_H

#include <pelican/core/AbstractModule.h>

#include <eigen3/Eigen/Dense>
#include <complex>
#include <fftw3.h>

using namespace pelican;
using namespace Eigen;

class StreamBlob;

class Imager : public AbstractModule
{
friend class ImagerTest;

public:
  Imager(const ConfigNode &inConfig);
  ~Imager();

  void run(const StreamBlob *input, StreamBlob *output);

private:
  void gridding(const MatrixXcf &correlations, const MatrixXf &uCoords, const MatrixXf &vCoords, const MatrixXf &mask, MatrixXcf &grid);
  void fftShift(MatrixXcf &matrix);

  MatrixXd mAntennaITRF;
  MatrixXf mUCoords;
  MatrixXf mVCoords;

  MatrixXcf mGridded;
  float mDuv;

  fftwf_plan mFFTWPlan;
};

PELICAN_DECLARE_MODULE(Imager)

#endif // IMAGER_MODULE_H
