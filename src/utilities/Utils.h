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

double julianDay(const double inTime);

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
  T r(0);
  for (int i = 0, n = inP.rows(); i < n; i++)
    r += std::pow(inX, n-i-1) * inP(i);

  return r;
}

/**
 * @brief
 * Transform spherical to cartesian coordinates
 */
template<typename T>
void spherical2cartesian(const Matrix<T, Dynamic, 1> &inTheta,
                         const Matrix<T, Dynamic, 1> &inPhi,
                         const T inRad,
                         Matrix<T, Dynamic, Dynamic> &outCart)
{
  Q_ASSERT(inTheta.rows() == inPhi.rows());
  Q_ASSERT(outCart.rows() == inTheta.rows());
  Q_ASSERT(outCart.cols() == 3);

  outCart.col(0) = inRad * inPhi.array().cos() * inTheta.array().cos(); // x
  outCart.col(1) = inRad * inPhi.array().cos() * inTheta.array().sin(); // y
  outCart.col(2) = inRad * inPhi.array().sin();                         // z
}

/**
 * @brief
 * Compute rotation matrix describing the precession from J2000 to the specified target time in Julian days
 */
template<typename T>
void precessionMatrix(const double inJD, Matrix<T, 3, 3> &outM)
{
  // Precession time in centuries
  double pt = (inJD - 2451545.0) / 36525.0;

  static const T eps = (23.0 + 26.0 / 60.0 + 21.448 / 3600.0) * M_PI / 180.0;
  static const Matrix<T, 4, 1> pc1(-0.001147, -1.07259, 5038.47875, 0.0);
  static const Matrix<T, 4, 1> pc2(-0.007726, 0.05127, -0.02524, (eps * 180.0 / M_PI) * 3600.0);
  static const Matrix<T, 4, 1> pc3(-0.001125, -2.38064, 10.5526, 0.0);

  T psi_a = (polyval<T>(pc1, pt) / 3600.0) * M_PI / 180.0;
  T omega_a = (polyval<T>(pc2, pt) / 3600.0) * M_PI / 180.0;
  T chi_a = (polyval<T>(pc3, pt) / 3600.0) * M_PI / 180.0;

  T s1 = sin(eps);
  T s2 = sin(-psi_a);
  T s3 = -sin(omega_a);
  T s4 = sin(chi_a);
  T c1 = cos(eps);
  T c2 = cos(-psi_a);
  T c3 = cos(-omega_a);
  T c4 = cos(chi_a);

  outM << c4*c2 - s2*s4*c3, c4*s2*c1 + s4*c3*c2*c1 - s1*s4*s3, c4*s2*s1 + s4*c3*c2*s1 + c1*s4*s3,
         -s4*c2 - s2*c4*c3, c4*c3*c2*c1 - s4*s2*c1 - s1*c4*s3, c4*c3*c2*s1 + c1*c4*s3 - s4*s2*s1,
          s2*s3, -s3*c2*c1 - s1*c3, c3*c1 - s3*c2*s1;
}
}

#endif // UTILITIES_H
