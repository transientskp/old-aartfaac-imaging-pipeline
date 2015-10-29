#ifndef FLAGGER_MODULE_H
#define FLAGGER_MODULE_H

#include <pelican/core/AbstractModule.h>
#include <eigen3/Eigen/Dense>
#include <vector>

using namespace pelican;
using namespace Eigen;

class StreamBlob;

class Flagger : public AbstractModule
{
public:
  Flagger(const ConfigNode &inConfig);
  ~Flagger();

  void run(const int pol, const StreamBlob *input, StreamBlob *output);

private:
  std::vector<int> ParseFlagged(const QString &s);
  std::vector<int> mFlaggedAnts;

  float mAntSigma;
  float mVisSigma;
  MatrixXf mAbs;
  MatrixXf mMask;
  VectorXf mStd;
  VectorXf mCentroid;
  VectorXf mMean;
  VectorXf mMinVal;
  VectorXf mMaxVal;
  VectorXf mAntennas;
  VectorXcf mResult;
};

PELICAN_DECLARE_MODULE(Flagger)

#endif // FLAGGER_MODULE_H
