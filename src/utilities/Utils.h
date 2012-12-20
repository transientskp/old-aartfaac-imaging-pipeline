#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>
#include <eigen3/Eigen/Dense>
#include <iostream>

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
 * Computes the Khatri-Rao (column wise Kronecker) product of A and B
 */
template<typename T>
void khatrirao(const Matrix<T, Dynamic, Dynamic> &inA, const Matrix<T, Dynamic, Dynamic> &inB, Matrix<T, Dynamic, Dynamic> &outC)
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
 * Form the kronecker product of two matrices, defined block by block
 */
template<typename T>
void kronecker(const Matrix<T, Dynamic, Dynamic> &inA, const Matrix<T, Dynamic, Dynamic> &inB, Matrix<T, Dynamic, Dynamic> &outC)
{
  Q_ASSERT(outC.cols() == inA.cols()*inB.cols());
  Q_ASSERT(outC.rows() == inA.rows()*inB.rows());

  for (int i = 0; i < inA.rows(); i++)
    for (int j = 0; j < inA.cols(); j++)
      outC.block(i*inB.rows(), j*inB.cols(), inB.rows(), inB.cols()) = inA(i,j) * inB;
}

/**
 * @brief
 * Evaluates a polynomial at the specified values of x
 */
template<typename T>
T polyval(const Matrix<T, Dynamic, 1> &inP, const T &inX)
{
  int n = inP.rows();
  Matrix<T, Dynamic, 1> A(n);
  for (int i = 0; i < n; i++)
    A(i) = std::pow(inX, n-i-1);

  return A.dot(inP);
}
}

#endif // UTILITIES_H
