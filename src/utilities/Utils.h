#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <typeinfo>

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
 * Compute the pseudo inverse using the singular value decomposition
 */
template<typename T>
void pseudoInverse(const Matrix<T, Dynamic, Dynamic> &inA, Matrix<T, Dynamic, Dynamic> &outI)
{
  Q_ASSERT(outI.rows() == inA.rows());
  Q_ASSERT(outI.cols() == inA.cols());
  Q_ASSERT(inA.rows() >= inA.cols());

  if (inA.cols() == 1)
  {
    outI = (inA.adjoint().array() / inA.squaredNorm()).transpose();
    return;
  }

  static JacobiSVD<Matrix<T, Dynamic, Dynamic> > svd;
  svd.compute(inA, ComputeFullU | ComputeFullV);

  outI.setZero(); // Use outI as temporary (Sigma^+)
  for (int i = 0, n = svd.singularValues().rows(); i < n; i++)
    outI(i, i) = 1.0 / svd.singularValues()(i);

  outI = (svd.matrixV() * outI.transpose() * svd.matrixU().adjoint());
}

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

template<typename Derived>
void toOctaveStderr(const DenseBase<Derived> &M, const char *name)
{
  if (M.size() == 0)
    return;

  const char *type = (typeid(M(0,0)) == typeid(std::complex<float>) || typeid(M(0,0)) == typeid(std::complex<double>)) ? "complex matrix" : "matrix";
  std::cerr << "# name: " << name << std::endl;
  std::cerr << "# type: " << type << std::endl;
  std::cerr << "# rows: " << M.rows() << std::endl;
  std::cerr << "# columns: " << M.cols() << std::endl;

  std::cerr.unsetf(std::ios::floatfield);
  std::cerr.precision(25);
  for (int i = 0; i < M.rows(); i++)
  {
    for (int j = 0; j < M.cols(); j++)
      std::cerr << M(i,j) << "\t";
    std::cerr << std::endl;
  }
}

/**
 * @brief
 * Conversion from source positions in radec to ITRF coordinates
 */
template<typename T>
void radec2itrf(const Matrix<T, Dynamic, 1> &inRa,
                const Matrix<T, Dynamic, 1> &inDec,
                const Matrix<int, Dynamic, 1> &inEpoch,
                const double inJD,
                Matrix<T, Dynamic, Dynamic> &outItrf)
{
  Q_ASSERT(inRa.rows() == inDec.rows());
  Q_ASSERT(outItrf.rows() == inRa.rows());
  Q_ASSERT(outItrf.cols() == 3);

  static Matrix<T, 3, 3> tmp_mat;

  int n = inRa.rows();
  Matrix<T, Dynamic, Dynamic> cartesian_mat(n, 3);
  spherical2cartesian<T>(inRa, inDec, 1, cartesian_mat);

  // convert B1950 to J2000
  if (inEpoch.sum() > 0)
  {
    precessionMatrix<T>(2433282.5, tmp_mat);
    for (int i = 0; i < n; i++)
      if (inEpoch(i) == 1)
        cartesian_mat.row(i) *= tmp_mat;
  }

  // compute apparent position
  precessionMatrix<T>(inJD, tmp_mat);
  Matrix<T, Dynamic, Dynamic> app_pos(n, 3);
  app_pos = cartesian_mat * tmp_mat.transpose();

  // Greenwich mean sidereal time in seconds
  static const Matrix<T, 4, 1> pc(-6.2e-6, 0.093104, 8640184.812866, 24110.54841);
  T tu = (floor(inJD) + 0.5 - 2451545.0) / 36525.0;
  T gmst = (inJD - floor(inJD) - 0.5) * 86400.0 * 1.002737811906 + polyval<T>(pc, tu);
  gmst = (gmst / 86400.0) * 2.0 * M_PI;


  tmp_mat << cos(gmst),-sin(gmst), 0.0,
             sin(gmst), cos(gmst), 0.0,
                   0.0,       0.0, 1.0;

  outItrf = app_pos * tmp_mat;
}
} // namespace utils

#endif // UTILITIES_H
