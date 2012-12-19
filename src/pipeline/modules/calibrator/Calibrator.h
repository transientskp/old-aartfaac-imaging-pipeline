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
  void khatrirao(const MatrixXcf &inA, const MatrixXcf &inB, MatrixXcf &outC);
  void kronecker(const MatrixXcf &inA, const MatrixXcf &inB, MatrixXcf &outC);
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
