#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>
#include <eigen3/Eigen/Dense>

using namespace Eigen;

namespace utils
{
/**
 * @brief
 * Converts a modified julian date in seconds to QDateTime
 */
QDateTime MJD2QDateTime(const double inMJD);

/**
 * @brief
 * Get the current system time in microseconds
 */
long GetTimeInMicros();

/**
 * @brief
 * Perform column-wise Khatri-Rao function
 */
template<class EigenMat>
void khatrirao(const EigenMat &inA, const EigenMat &inB, EigenMat &outC)
{
  Q_ASSERT(inA.cols() == inB.cols());
  Q_ASSERT(outC.rows() == inA.rows()*inB.rows());
  Q_ASSERT(outC.cols() == inA.cols());

  for (int i = 0; i < inA.rows(); i++)
    for (int j = 0; j < inA.cols(); j++)
      outC.block(i*inB.rows(), j, inB.rows(), 1) = inA(i,j) * inB.col(j);
}

/**
 * @brief
 * Perform the Kronecker function
 */
template<class EigenMat>
void kronecker(const EigenMat &inA, const EigenMat &inB, EigenMat &outC)
{
  Q_ASSERT(outC.cols() == inA.cols()*inB.cols());
  Q_ASSERT(outC.rows() == inA.rows()*inB.rows());

  for (int i = 0; i < inA.rows(); i++)
    for (int j = 0; j < inA.cols(); j++)
      outC.block(i*inB.rows(), j*inB.cols(), inB.rows(), inB.cols()) = inA(i,j) * inB;
}
}

#endif // UTILITIES_H
