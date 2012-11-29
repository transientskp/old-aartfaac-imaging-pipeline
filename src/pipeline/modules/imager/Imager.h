#ifndef IMAGER_MODULE_H
#define IMAGER_MODULE_H

#include <pelican/core/AbstractModule.h>

#include <eigen3/Eigen/Dense>
#include <eigen3/unsupported/Eigen/FFT>

using namespace pelican;
using namespace Eigen;

class StreamBlob;

class Imager : public AbstractModule
{
public:
  Imager(const ConfigNode &inConfig);
  ~Imager();

  void run(const StreamBlob *input, StreamBlob *output);

private:
  void gridding(const MatrixXcf &inCorrelations, const std::vector<int> &inFlagged);

  MatrixXf mUCoords;
  MatrixXf mVCoords;

  MatrixXcf mGridded;
};

PELICAN_DECLARE_MODULE(Imager)

#endif // IMAGER_MODULE_H
