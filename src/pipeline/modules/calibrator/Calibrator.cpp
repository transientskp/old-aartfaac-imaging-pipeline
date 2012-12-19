#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"

#include <pelican/utility/Config.h>
#include <QtCore>

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

void Calibrator::kronecker(const MatrixXcf &inA, const MatrixXcf &inB, MatrixXcf &outC)
{
  Q_ASSERT(outC.cols() == inA.cols()*inB.cols());
  Q_ASSERT(outC.rows() == inA.rows()*inB.rows());

  for (int i = 0; i < inA.rows(); i++)
    for (int j = 0; j < inA.cols(); j++)
      outC.block(i*inB.rows(), j*inB.cols(), inB.rows(), inB.cols()) = inA(i,j) * inB;
}

void Calibrator::khatrirao(const MatrixXcf &inA, const MatrixXcf &inB, MatrixXcf &outC)
{
  Q_ASSERT(inA.cols() == inB.cols());
  Q_ASSERT(outC.rows() == inA.rows()*inB.rows());
  Q_ASSERT(outC.cols() == inA.cols());

  for (int j = 0; j < inA.cols(); j++)
    for (int i = 0; i < inA.rows(); i++)
      outC.block(i*inB.rows(), j, inB.rows(), 1) = inA(i,j) * inB.col(j);
}
