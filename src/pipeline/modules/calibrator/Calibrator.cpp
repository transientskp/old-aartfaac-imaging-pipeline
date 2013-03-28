#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"
#include "../../../utilities/AntennaPositions.h"
#include "../../../utilities/Utils.h"
#include "../../../utilities/NMSMax.h"

#include <pelican/utility/Config.h>
#include <QtCore>
#include <QDebug>


Calibrator::Calibrator(const ConfigNode &inConfig):
  AbstractModule(inConfig)
{
  mAntennaITRFReshaped = ANT_ITRF();

  mMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mSpatialFilterMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mNoiseCovMatrix.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  MatrixXd u = ANT_U(), v = ANT_V(), w = ANT_W();
  mUVDist = (u.array().square() + v.array().square() + w.array().square()).sqrt();

  mRaSources.resize(4);
  mDecSources.resize(4);
  mFluxes.resize(4);

  // Cassiopeia A (11000 flux)
  mRaSources(0)   = 6.113786558863104;
  mDecSources(0)  = 1.021919365863547;

  // Cygnus A (8100 flux)
  mRaSources(1)   = 5.226192095556169;
  mDecSources(1)  = 0.7086036763096979;

  // Tauras A (1420 flux)
  mRaSources(2)   = 1.446441617590301;
  mDecSources(2)  = 0.3835070143048873;

  // Virgo A (970 flux)
  mRaSources(3)   = 3.265074698168392;
  mDecSources(3)  = 0.221104127406815;

  mEpoch.resize(4);
  mEpoch.setOnes();

  mNormalizedData.resize(NUM_ANTENNAS, NUM_ANTENNAS);
}

Calibrator::~Calibrator()
{
}

void Calibrator::run(const StreamBlob *input, StreamBlob *output)
{
  static const double min_restriction = 10.0;                 ///< avoid vis. below this wavelength
  static const double max_restriction = 60.0;                 ///< avoid vis. above this much meters
  static const Vector3d normal(0.598753, 0.072099, 0.797682); ///< Normal to CS002 (central antenna)

  mFrequency = input->mFrequency;
  double uvdist_cutoff = std::min(min_restriction*(C_MS/mFrequency), max_restriction);

  // =====================================
  // ==== 0. Prepare/Reshape matrices ====
  // =====================================
  int num_antennas = NUM_ANTENNAS - input->mFlagged.size();

  if (mFlagged.size() != input->mFlagged.size())
  {
    mNormalizedData.resize(num_antennas, num_antennas);
    mSpatialFilterMask.resize(num_antennas, num_antennas);
    mMask.resize(num_antennas, num_antennas);
    mNoiseCovMatrix.resize(num_antennas, num_antennas);
    mAntennaITRFReshaped.resize(num_antennas, 3);
    mGains.resize(num_antennas);
  }
  mFlagged = input->mFlagged;

  for (int a1 = 0, _a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    if (std::find(mFlagged.begin(), mFlagged.end(), a1) != mFlagged.end())
      continue;

    mAntennaITRFReshaped.row(_a1) = ANT_ITRF().row(a1);

    for (int a2 = 0, _a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      if (std::find(mFlagged.begin(), mFlagged.end(), a2) != mFlagged.end())
        continue;

      mNormalizedData(_a1, _a2) = input->mXX(a1, a2);
      mSpatialFilterMask(_a1, _a2) = mUVDist(a1, a2) < uvdist_cutoff ? 1.0f : 0.0f;
      mMask(_a1, _a2) = input->mMask(a1, a2);
      _a2++;
    }

    _a1++;
  }

  double time = input->mMJDTime / 86400.0 + 2400000.5;

  // ========================================================================
  // ==== 1. Whitening of the array covariance matrix for DOA estimation ====
  // ========================================================================
  mNormalizedData.array() /= (mNormalizedData.diagonal() * mNormalizedData.diagonal().transpose()).array().sqrt();

  // ================================
  // ==== 2. Initial calibration ====
  // ================================
  MatrixXd src_pos(mRaSources.rows(), 3);
  utils::radec2itrf<double>(mRaSources, mDecSources, mEpoch, time, src_pos);
  VectorXd up = src_pos * normal;
  mSelection.resize((up.array() > 0.0).count(), 3);

  for (int i = 0, j = 0, n = src_pos.rows(); i < n; i++)
    if (up(i) > 0.0)
    {
      for (int k = 0; k < src_pos.cols(); k++)
        mSelection(j,k) = src_pos(i,k);
      j++;
    }
  statCal(mNormalizedData, mFrequency, mMask, mGains, mFluxes, mNoiseCovMatrix);

  // ====================================
  // ==== 3. WSF Position Estimation ====
  // ====================================
  Q_ASSERT(mSelection.rows() == mFluxes.rows());
  MatrixXd selection((mFluxes.array() > 0.01f).count(), 3);
  VectorXf fluxes(selection.rows());
  for (int i = 0, j = 0, n = selection.rows(); i < n; i++)
    if (mFluxes(i) > 0.01f)
    {
      selection.row(j) = mSelection.row(i);
      fluxes(j) = mFluxes(i);
      j++;
    }
  wsfSrcPos(mNormalizedData, mNoiseCovMatrix, mGains, mFrequency, selection);

  // ==============================
  // ==== 4. Final calibration ====
  // ==============================
  std::complex<float> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * mFrequency / C_MS;
  MatrixXcf A = (-i1 * (mAntennaITRFReshaped * selection.transpose())).array().exp().cast<std::complex<float> >();
  MatrixXf inv_mask = (mNoiseCovMatrix.array().abs() > 0.0).select(MatrixXf::Zero(mNoiseCovMatrix.rows(), mNoiseCovMatrix.cols()), 1.0);
  walsCalibration(A, mNormalizedData, fluxes, inv_mask, mGains, mFluxes, mNoiseCovMatrix);
  mGains = (1.0/mGains.array());
  mGains.adjointInPlace();
  mNormalizedData = (mGains.transpose().adjoint() * mGains.transpose()).array() * (mNormalizedData.array() - mNoiseCovMatrix.array()).array();

  // ===============================
  // ==== 5. A-team subtraction ====
  // ===============================
  MatrixXcf ATeam = A * mFluxes.asDiagonal() * A.adjoint();
  mNormalizedData.array() -= ATeam.array();

  // ================================================================
  // ==== 6. Reconstruct the full ACM from the reshaped matrices ====
  // ================================================================
  for (int a1 = 0, _a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    if (std::find(mFlagged.begin(), mFlagged.end(), a1) != mFlagged.end())
      continue;

    for (int a2 = 0, _a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      if (std::find(mFlagged.begin(), mFlagged.end(), a2) != mFlagged.end())
        continue;

      output->mXX(a1, a2) = mNormalizedData(_a1, _a2);
      _a2++;
    }

    _a1++;
  }
}

void Calibrator::statCal(const MatrixXcf &inData,
                         const double inFrequency,
                         MatrixXf &ioMask,
                         VectorXcf &outCalibrations,
                         VectorXf &outSigmas,
                         MatrixXcf &outVisibilities)
{
  std::complex<float> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * inFrequency / C_MS;
  MatrixXcf A = (-i1 * (mAntennaITRFReshaped * mSelection.transpose())).array().exp().cast<std::complex<float> >();

  MatrixXcf KA(A.rows()*A.rows(), A.cols());
  utils::khatrirao<std::complex<float> >(A.conjugate(), A, KA);

  MatrixXf AA = (A.adjoint() * A).array().abs().square();

  MatrixXf mask = 1.0f - (ioMask.array() > mSpatialFilterMask.array()).select(ioMask, mSpatialFilterMask).array();
  MatrixXcf data = inData.array() * mask.array();
  data.resize(inData.rows()*inData.cols(), 1);
  VectorXf flux = (AA.inverse() * KA.adjoint() * data).array().real();
  flux.array() /= flux(0);
  flux = (flux.array() < 0.0f).select(0.0f, flux);

  walsCalibration(A, inData, flux, mask, outCalibrations, outSigmas, outVisibilities);
  outCalibrations = (1.0f/outCalibrations.array()).conjugate();
}


int Calibrator::walsCalibration(const MatrixXcf &inModel,  					// A
                                const MatrixXcf &inData,   					// Rhat
                                const VectorXf  &inFluxes, 					// sigmas
                                const MatrixXf  &inInvMask,         // mask
                                      VectorXcf &outGains,          // g
                                      VectorXf  &outSourcePowers,   // sigmas
                                      MatrixXcf &outNoiseCovMatrix) // Sigma_n
{
  static const int max_iterations = 50;
  static const float epsilon = 1e-3;

  Q_ASSERT(outNoiseCovMatrix.rows() == inData.rows());
  Q_ASSERT(outNoiseCovMatrix.cols() == inData.cols());

  outNoiseCovMatrix.setZero();
  VectorXcf cur_gains(inData.rows());
  VectorXcf prev_gains(inData.rows());
  for (int i = 0; i < inData.rows(); i++)
    prev_gains(i) = std::complex<float>(0.0, 1.0f);
  VectorXf cur_fluxes(inFluxes.rows());
  VectorXf prev_fluxes(inFluxes);

  int n = (inInvMask.array() > 0.5).count();
  MatrixXcf rest(n, 1);
  MatrixXcf data(n, 1);
  MatrixXcf pinv(n, 1);
  int i;
  for (i = 1; i <= max_iterations; i++)
  {
    // ======================================================
    // ==== 1. Per sensor gain estimation using gainSolv ====
    // ======================================================
    MatrixXcf M = inModel * prev_fluxes.asDiagonal() * inModel.adjoint();
    gainSolv(M.array() * inInvMask.array(), inData.array() * inInvMask.array(), prev_gains, cur_gains);

    MatrixXcf GA = cur_gains.asDiagonal() * inModel;
    MatrixXcf Rest = GA * prev_fluxes.asDiagonal() * GA.adjoint();

    Q_ASSERT(Rest.size() == inInvMask.size());

    int j = 0, k = 0;
    while (j < Rest.size())
    {
      if (inInvMask(j) > 0.5)
      {
        rest(k) = Rest(j);
        data(k) = inData(j);
        k++;
      }
      j++;
    }

    utils::pseudoInverse<std::complex<float> >(rest, pinv);
    MatrixXcf Y = pinv.transpose() * data;
    Q_ASSERT(Y.size() == 1);
    float normg = std::abs(std::sqrt(Y(0)));
    cur_gains = normg * cur_gains / (cur_gains(0) / std::abs(cur_gains(0)));
    cur_gains = (cur_gains.array().real() == INFINITY || cur_gains.array().imag() == INFINITY).select(1, cur_gains);

    // =========================================
    // ==== 2. Model source flux estimation ====
    // =========================================
    MatrixXcf inv_data = inData.inverse();
    GA = cur_gains.asDiagonal() * inModel;
    MatrixXf lhs = (GA.adjoint() * inv_data * GA).conjugate().array().abs().square();
    MatrixXcf rhs = (GA.adjoint() * inv_data * (inData - outNoiseCovMatrix) * inv_data * GA).diagonal();
    cur_fluxes = (lhs.inverse() * rhs).array().real();

    if ((cur_fluxes.array() == INFINITY).any())
      cur_fluxes = prev_fluxes;

    Q_ASSERT(cur_fluxes(0) != 0.0);
    cur_fluxes /= cur_fluxes(0);
    cur_fluxes = (cur_fluxes.array() >= 0.0).select(cur_fluxes, 0.0);

    // ========================================
    // ==== 3. Noise covariance estimation ====
    // ========================================
    outNoiseCovMatrix = (inData - GA * cur_fluxes.asDiagonal() * GA.adjoint()).array() * (1.0f - inInvMask.array());

    // =================================
    // ==== 4. Test for convergence ====
    // =================================
    MatrixXcf prev_theta(prev_gains.size() + prev_fluxes.size(), 1);
    MatrixXcf cur_theta(cur_gains.size() + cur_fluxes.size(), 1);
    for (int j = 0; j < prev_gains.size(); j++)
    {
      prev_theta(j) = prev_gains(j);
      cur_theta(j) = cur_gains(j);
    }
    for (int j = 0; j < prev_fluxes.size(); j++)
    {
      prev_theta(j+prev_gains.size()) = std::complex<float>(prev_fluxes(j), 0.0);
      cur_theta(j+prev_gains.size()) = std::complex<float>(cur_fluxes(j), 0.0);
    }

    MatrixXcf pinv(prev_theta.rows(), prev_theta.cols());
    utils::pseudoInverse<std::complex<float> >(prev_theta, pinv);
    MatrixXcf X = pinv.transpose() * cur_theta;
    Q_ASSERT(X.size() == 1);

    float x = std::abs(X(0) - 1.0f);
    if (x < epsilon)
    {
      qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
      break;
    }

    // ================================
    // ==== Prepare next iteration ====
    // ================================
    prev_gains = cur_gains;
    prev_fluxes = cur_fluxes;
  }

  outGains = cur_gains;
  outSourcePowers = cur_fluxes;

  if (i >= max_iterations)
    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);

  return i;
}

int Calibrator::gainSolv(const MatrixXcf &inModel,
                         const MatrixXcf &inData,
                         const VectorXcf &inEstimatedGains,
                               VectorXcf &outGains)
{
  Q_ASSERT(inModel.rows() == inModel.cols());
  Q_ASSERT(inData.rows() == inData.cols());
  Q_ASSERT(inModel.rows() == inData.rows());
  Q_ASSERT(inModel.rows() == inEstimatedGains.rows());
  Q_ASSERT(inEstimatedGains.rows() == outGains.rows());

  int n = inModel.rows();

  // allocate once
  MatrixXcf data_normalised(n, n);
  MatrixXcf data_calibrated(n, n);
  VectorXcf estimated_calibration(n);
  VectorXcf tmp(n);

  static const int max_iterations = 100;
  static const float epsilon = 1e-6f;

  for (int i = 0; i < n; i++)
  {
    // Normalize the data
    data_normalised.col(i) = inData.col(i) / inData.col(i).dot(inData.col(i));

    // Initial calibration
    data_calibrated.col(i) = inEstimatedGains.array() * inModel.col(i).array();
  }

  int i;
  for (i = 1; i <= max_iterations; i++)
  {
    for (int j = 0; j < n; j++)
    {
      estimated_calibration(j) = data_normalised.col(j).dot(data_calibrated.col(j));
      outGains(j) = 1.0f / std::conj(estimated_calibration(j));
    }

    if (i % 2 == 1)
      tmp = outGains;
    else
    {
      // Update gains and check for convergence
      estimated_calibration = outGains;
      outGains = (outGains.array() + tmp.array()) / 2.0;
      float gains_normal = outGains.norm();
      tmp = outGains.array() - estimated_calibration.array();
      float delta_gains_normal = tmp.norm();
      if (delta_gains_normal / gains_normal <= epsilon)
      {
        qDebug("[%s] Convergence after %d iterations", __FUNCTION__, i);
        break;
      }
    }

    // Update calibration of array covariance matrix
    for (int j = 0; j < n; j++)
      data_calibrated.col(j) = outGains.array() * inModel.col(j).array();
  }

  if (i >= max_iterations)
    qCritical("[%s] No convergence after %d iterations", __FUNCTION__, i);

  return i;
}


void Calibrator::wsfSrcPos(const MatrixXcf &inData,
                           const MatrixXcf &inSigma1,
                           const VectorXcf &inGains,
                           const double inFreq,
                                 MatrixXd &ioPositions)
{
  int nsrc = ioPositions.rows();
  VectorXd init(nsrc*2);
  for (int i = 0; i < nsrc; i++)
  {
    init(i) = atan(ioPositions(i,1) / ioPositions(i,0));
    init(i + nsrc) = asin(ioPositions(i,2));
  }

  ComplexEigenSolver<MatrixXcf> solver(inData);

  VectorXf eigenvalues_abs = solver.eigenvalues().array().abs();
  VectorXi I = NM::sort(eigenvalues_abs);

  MatrixXcf Es(solver.eigenvectors().rows(), nsrc);
  MatrixXf eigenmat_abs(nsrc, nsrc);
  eigenmat_abs.setZero();
  for (int i = 0; i < nsrc; i++)
  {
    eigenmat_abs(i,i) = eigenvalues_abs(i);
    Es.col(i) = solver.eigenvectors().col(I(i));
  }

  MatrixXf eye = MatrixXf::Identity(nsrc, nsrc);
  MatrixXf S = inSigma1.diagonal().array().real();
  MatrixXf A = (eigenmat_abs.array() - (S.mean() * eye).array());
  MatrixXf W = (A * A).array() / eigenmat_abs.array();
  W = (W.array() != W.array()).select(0,W);
  MatrixXcf EsWEs = Es * W * Es.adjoint();
  MatrixXcf T = 1.0f / inGains.array();
  MatrixXcf G = T.conjugate().asDiagonal().toDenseMatrix();

  WSFCost wsf_cost(EsWEs, G, inFreq, mAntennaITRFReshaped);

  init = NM::Simplex(wsf_cost, init, 1e-4);

  ioPositions.col(0) = init.head(nsrc).array().cos() * init.tail(nsrc).array().cos();
  ioPositions.col(1) = init.head(nsrc).array().sin() * init.tail(nsrc).array().cos();
  ioPositions.col(2) = init.tail(nsrc).array().sin();
}

float Calibrator::WSFCost::operator()(const VectorXd &theta)
{
    const int nsrc = theta.size() / 2;
    const int nelem = P.rows();

    MatrixXd src_pos(nsrc, 3);
    src_pos.col(0) = theta.head(nsrc).array().cos() * theta.tail(nsrc).array().cos();
    src_pos.col(1) = theta.head(nsrc).array().sin() * theta.tail(nsrc).array().cos();
    src_pos.col(2) = theta.tail(nsrc).array().sin();

    std::complex<double> i1(0.0, 1.0);
    i1 *= 2.0 * M_PI * freq / C_MS;
    MatrixXcf T = (-i1 * (P * src_pos.transpose())).array().exp().cast<std::complex<float> >();
    MatrixXcf A = G * T;
    MatrixXcf PAperp = MatrixXcf::Identity(nelem, nelem).array() - (A * (A.adjoint() * A).inverse() * A.adjoint()).array();

    return (PAperp * W).trace().real();
}

