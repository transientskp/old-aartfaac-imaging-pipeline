#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"
#include "../../../utilities/UVWParser.h"
#include "../../../utilities/Utils.h"
#include "../../../utilities/Simplex.h"

#include <algorithm>
#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/utility/Config.h>
#include <QtCore>
#include <QDebug>

extern char *gTableName;


Calibrator::Calibrator(const ConfigNode &inConfig):
  AbstractModule(inConfig)
{
  casa::MeasurementSet ms(gTableName);
  casa::ROMSColumns msc(ms);

  mAntennaITRF.resize(NUM_ANTENNAS, 3);
  mAntennaITRFReshaped.resize(NUM_ANTENNAS, 3);
  for (int a = 0; a < NUM_ANTENNAS; a++)
  {
    mAntennaITRF(a, 0) = msc.antenna().position()(a)(casa::IPosition(1,0));
    mAntennaITRF(a, 1) = msc.antenna().position()(a)(casa::IPosition(1,1));
    mAntennaITRF(a, 2) = msc.antenna().position()(a)(casa::IPosition(1,2));
    mAntennaITRFReshaped.row(a) = mAntennaITRF.row(a);
  }

  mMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mUVDist.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mSpatialFilterMask.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mNoiseCovMatrix.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  QString uvw_file_name = inConfig.getOption("uvw", "path");
  UVWParser::Type lba_type = UVWParser::Type(inConfig.getOption("lba", "type").toInt());
  UVWParser uvw_parser(uvw_file_name);
  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      casa::String a1_name = msc.antenna().name()(a1);
      casa::String a2_name = msc.antenna().name()(a2);

      UVWParser::UVW uvw = uvw_parser.GetUVW(a1_name.c_str(),
                                             a2_name.c_str(),
                                             lba_type);

      mUCoords(a1, a2) = uvw.uvw[0];
      mVCoords(a1, a2) = uvw.uvw[1];
    }
  }

  mUVDist = (mUCoords.array().square() + mVCoords.array().square()).sqrt();

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
  static const double min_restriction = 10.0;   									 ///< avoid vis. below this wavelengths
  static const double max_restriction = 60.0;   									 ///< avoid vis. above this much meters
  static const Vector3d normal(0.598753, 0.072099, 0.797682); ///< Normal to CS002 (central antenna)

  mFrequency = input->mFrequency;
  double uvdist_cutoff = std::min<double>(min_restriction*(C_MS/mFrequency), max_restriction);

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

    mAntennaITRFReshaped.row(_a1) = mAntennaITRF.row(a1);

    for (int a2 = 0, _a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      if (std::find(mFlagged.begin(), mFlagged.end(), a2) != mFlagged.end())
        continue;

      mNormalizedData(_a1, _a2) = input->mXX(a1, a2);
      mSpatialFilterMask(_a1, _a2) = mUVDist(a1, a2) < uvdist_cutoff ? 1.0 : 0.0;
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
  utils::matrix2stderr(mFluxes, "fluxes");

  // ====================================
  // ==== 3. WSF Position Estimation ====
  // ====================================
  Q_ASSERT(mSelection.rows() == mFluxes.rows());
  MatrixXd selection((mFluxes.array() > 0.01).count(), 3);
  VectorXd fluxes(selection.rows());
  for (int i = 0, j = 0, n = selection.rows(); i < n; i++)
    if (mFluxes(i) > 0.01)
    {
      selection.row(j) = mSelection.row(i);
      fluxes(j) = mFluxes(i);
      j++;
    }

  wsfSrcPos(mNormalizedData, mNoiseCovMatrix, mGains, mFrequency, selection);

  // ==============================
  // ==== 4. Final calibration ====
  // ==============================
  std::complex<double> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * mFrequency / C_MS;
  MatrixXcd A = (-i1 * (mAntennaITRFReshaped * selection.transpose())).array().exp();
  MatrixXd inv_mask = (mNoiseCovMatrix.array().abs() > 0.0).select(MatrixXd::Zero(mNoiseCovMatrix.rows(), mNoiseCovMatrix.cols()), 1.0);
  walsCalibration(A, mNormalizedData, fluxes, inv_mask, mGains, mFluxes, mNoiseCovMatrix);
  mGains = (1.0/mGains.array());
  mGains.adjointInPlace();
  mNormalizedData = (mGains * mGains.adjoint()).array() * (mNormalizedData.array() - mNoiseCovMatrix.array()).array();


  // ===============================
  // ==== 5. A-team subtraction ====
  // ===============================
  MatrixXcd ATeam = A * mFluxes.asDiagonal() * A.adjoint();
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

void Calibrator::statCal(const MatrixXcd &inData,
                         const double inFrequency,
                         MatrixXd &ioMask,
                         VectorXcd &outCalibrations,
                         VectorXd &outSigmas,
                         MatrixXcd &outVisibilities)
{
  std::complex<double> i1(0.0, 1.0);
  i1 *= 2.0 * M_PI * inFrequency / C_MS;
  MatrixXcd A = (-i1 * (mAntennaITRFReshaped * mSelection.transpose())).array().exp();

  MatrixXcd KA(A.rows()*A.rows(), A.cols());
  utils::khatrirao<std::complex<double> >(A.conjugate(), A, KA);

  MatrixXd AA = (A.adjoint() * A).array().abs().square();

  MatrixXd mask = 1 - (ioMask.array() > mSpatialFilterMask.array()).select(ioMask, mSpatialFilterMask).array();
  MatrixXcd data = inData.array() * mask.array();
  data.resize(inData.rows()*inData.cols(), 1);
  VectorXd flux = (AA.inverse() * KA.adjoint() * data).array().real();
  flux.array() /= flux(0);
  flux = (flux.array() < 0.0).select(0.0, flux);

  walsCalibration(A, inData, flux, mask, outCalibrations, outSigmas, outVisibilities);
  outCalibrations = (1.0/outCalibrations.array()).conjugate();
}


int Calibrator::walsCalibration(const MatrixXcd &inModel,  					// A
                                const MatrixXcd &inData,   					// Rhat
                                const VectorXd  &inFluxes, 					// sigmas
                                const MatrixXd  &inInvMask,         // mask
                                      VectorXcd &outGains,          // g
                                      VectorXd  &outSourcePowers,   // sigmas
                                      MatrixXcd &outNoiseCovMatrix) // Sigma_n
{
  static const int max_iterations = 50;
  static const double epsilon = 1e-3;

  Q_ASSERT(outNoiseCovMatrix.rows() == inData.rows());
  Q_ASSERT(outNoiseCovMatrix.cols() == inData.cols());

  outNoiseCovMatrix.setZero();
  VectorXcd cur_gains(inData.rows());
  VectorXcd prev_gains(inData.rows());
  for (int i = 0; i < inData.rows(); i++)
    prev_gains(i) = std::complex<double>(0.0, 1.0);
  VectorXd cur_fluxes(inFluxes.rows());
  VectorXd prev_fluxes(inFluxes);

  int n = (inInvMask.array() > 0.5).count();
  MatrixXcd rest(n, 1);
  MatrixXcd data(n, 1);
  MatrixXcd pinv(n, 1);
  int i;
  for (i = 1; i <= max_iterations; i++)
  {
    // ======================================================
    // ==== 1. Per sensor gain estimation using gainSolv ====
    // ======================================================
    MatrixXcd M = inModel * prev_fluxes.asDiagonal() * inModel.adjoint();
    gainSolv(M.array() * inInvMask.array(), inData.array() * inInvMask.array(), prev_gains, cur_gains);

    MatrixXcd GA = cur_gains.asDiagonal() * inModel;
    MatrixXcd Rest = GA * prev_fluxes.asDiagonal() * GA.adjoint();

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

    utils::pseudoInverse<std::complex<double> >(rest, pinv);
    MatrixXcd Y = pinv.transpose() * data;
    Q_ASSERT(Y.size() == 1);
    double normg = std::abs(std::sqrt(Y(0)));
    cur_gains = normg * cur_gains / (cur_gains(0) / std::abs(cur_gains(0)));
    cur_gains = (cur_gains.array().real() == INFINITY || cur_gains.array().imag() == INFINITY).select(1, cur_gains);

    // =========================================
    // ==== 2. Model source flux estimation ====
    // =========================================
    MatrixXcd inv_data = inData.inverse();
    GA = cur_gains.asDiagonal() * inModel;
    MatrixXd lhs = (GA.adjoint() * inv_data * GA).conjugate().array().abs().square();
    MatrixXcd rhs = (GA.adjoint() * inv_data * (inData - outNoiseCovMatrix) * inv_data * GA).diagonal();
    cur_fluxes = (lhs.inverse() * rhs).array().real();

    if ((cur_fluxes.array() == INFINITY).any())
      cur_fluxes = prev_fluxes;

    Q_ASSERT(cur_fluxes(0) != 0.0);
    cur_fluxes /= cur_fluxes(0);
    cur_fluxes = (cur_fluxes.array() >= 0.0).select(cur_fluxes, 0.0);

    // ========================================
    // ==== 3. Noise covariance estimation ====
    // ========================================
    outNoiseCovMatrix = (inData - GA * cur_fluxes.asDiagonal() * GA.adjoint()).array() * (1.0 - inInvMask.array());

    // =================================
    // ==== 4. Test for convergence ====
    // =================================
    MatrixXcd prev_theta(prev_gains.size() + prev_fluxes.size(), 1);
    MatrixXcd cur_theta(cur_gains.size() + cur_fluxes.size(), 1);
    for (int j = 0; j < prev_gains.size(); j++)
    {
      prev_theta(j) = prev_gains(j);
      cur_theta(j) = cur_gains(j);
    }
    for (int j = 0; j < prev_fluxes.size(); j++)
    {
      prev_theta(j+prev_gains.size()) = std::complex<double>(prev_fluxes(j), 0.0);
      cur_theta(j+prev_gains.size()) = std::complex<double>(cur_fluxes(j), 0.0);
    }

    MatrixXcd pinv(prev_theta.rows(), prev_theta.cols());
    utils::pseudoInverse<std::complex<double> >(prev_theta, pinv);
    MatrixXcd X = pinv.transpose() * cur_theta;
    Q_ASSERT(X.size() == 1);

    double x = std::abs(X(0) - 1.0);
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

int Calibrator::gainSolv(const MatrixXcd &inModel,
                         const MatrixXcd &inData,
                         const VectorXcd &inEstimatedGains,
                               VectorXcd &outGains)
{
  Q_ASSERT(inModel.rows() == inModel.cols());
  Q_ASSERT(inData.rows() == inData.cols());
  Q_ASSERT(inModel.rows() == inData.rows());
  Q_ASSERT(inModel.rows() == inEstimatedGains.rows());
  Q_ASSERT(inEstimatedGains.rows() == outGains.rows());

  int n = inModel.rows();

  // allocate once
  MatrixXcd data_normalised(n, n);
  MatrixXcd data_calibrated(n, n);
  VectorXcd estimated_calibration(n);
  VectorXcd tmp(n);

  static const int max_iterations = 100;
  static const double epsilon = 1e-6;

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
      outGains(j) = 1.0 / std::conj(estimated_calibration(j));
    }

    if (i % 2 == 1)
      tmp = outGains;
    else
    {
      // Update gains and check for convergence
      estimated_calibration = outGains;
      outGains = (outGains.array() + tmp.array()) / 2.0;
      double gains_normal = outGains.norm();
      tmp = outGains.array() - estimated_calibration.array();
      double delta_gains_normal = tmp.norm();
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

VectorXd gEigenValuesAbs;
bool EigenSorter(const int a, const int b)
{
  return gEigenValuesAbs(a) > gEigenValuesAbs(b);
}

void Calibrator::wsfSrcPos(const MatrixXcd &inData,
                           const MatrixXcd &inSigma1,
                           const VectorXcd &inGains,
                           const double inFreq,
                                 MatrixXd &ioPositions)
{
  int nsrc = ioPositions.rows();
  VectorXd elevation = ioPositions.col(2).array().asin();
  VectorXd azimuth = ioPositions.col(1).array() / ioPositions.col(0).array();
  for (int i = 0; i < nsrc; i++)
    azimuth(i) = atan(azimuth(i));

  ComplexEigenSolver<MatrixXcd> solver(inData);

  gEigenValuesAbs = solver.eigenvalues().array().abs();
  std::vector<int> indices(gEigenValuesAbs.size());
  for (int i = 0, n = indices.size(); i < n; i++)
    indices[i] = i;

  std::sort(indices.begin(), indices.end(), EigenSorter);

  MatrixXcd Es(solver.eigenvectors().rows(), nsrc);
  MatrixXcd eigenvalues_abs(nsrc, nsrc);
  eigenvalues_abs.setZero();
  for (int i = 0; i < nsrc; i++)
  {
    eigenvalues_abs(i,i).real() = gEigenValuesAbs(indices[i]);
    Es.col(i) = solver.eigenvectors().col(indices[i]);
  }

  MatrixXcd eye(nsrc, nsrc); eye.setIdentity();
  MatrixXcd A = (eigenvalues_abs.array() - (inSigma1.diagonal().mean() * eye).array());
  MatrixXcd W = (A * A).array() / eigenvalues_abs.array();
  MatrixXcd EsWEs = Es * W * Es.adjoint();
  MatrixXcd T = (inGains.array().inverse());
  MatrixXcd G = T.conjugate().asDiagonal().toDenseMatrix();
  WSFCost wsf_cost(EsWEs, G, inFreq, mAntennaITRFReshaped);
  vector<double> init(nsrc*2);
  for (int i = 0; i < nsrc; i++)
  {
    init[i] = azimuth(i);
    init[i+nsrc] = elevation(i);
  }

  init = BT::Simplex(wsf_cost, init);

  for (int i = 0; i < nsrc; i++)
  {
    ioPositions(i, 0) = cosf(init[i]) * cosf(init[i+nsrc]);
    ioPositions(i, 1) = sinf(init[i]) * cosf(init[i+nsrc]);
    ioPositions(i, 2) = sinf(init[i+nsrc]);
  }
}
