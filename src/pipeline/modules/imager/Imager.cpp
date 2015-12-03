#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../utilities/AntennaPositions.h"
#include "../../../utilities/Utils.h"
#include "Constants.h"

#include <pelican/utility/Config.h>
#include <QtCore>
#include <limits>

Imager::Imager(const ConfigNode &inConfig):
 AbstractModule(inConfig),
 mDuv(2.5f)
{
  AntennaPositions::CreateInstance(inConfig.getOption("positrf", "path"));
  mGridded.resize(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE);

  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mUCoords.setZero();
  mVCoords.setZero();

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      Vector3d p1 = ANT_XYZ(a1);
      Vector3d p2 = ANT_XYZ(a2);

      mUCoords(a1, a2) = p1(0) - p2(0);
      mVCoords(a1, a2) = p1(1) - p2(1);
    }
  }

  mMask.resize(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE);
  int h = IMAGE_OUTPUT_SIZE/2;
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      mMask(i,j) = (i-h)*(i-h) + (j-h)*(j-h);

  // Create fftw plan
  mFFTWPlan = fftwf_plan_dft_2d(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE,
                               reinterpret_cast<fftwf_complex*>(mGridded.data()),
                               reinterpret_cast<fftwf_complex*>(mGridded.data()),
                               FFTW_FORWARD, FFTW_ESTIMATE|FFTW_PRESERVE_INPUT);

  #ifndef NDEBUG
  std::cout << "BEGIN FFTW PLAN" << std::endl;
  fftwf_print_plan(mFFTWPlan);
  std::cout << std::endl << "END FFTW PLAN" << std::endl;
  #endif
}

Imager::~Imager()
{
  fftwf_destroy_plan(mFFTWPlan);
  fftwf_cleanup();
}

void Imager::run(const StreamBlob *input, StreamBlob *output)
{
  mDuv = C_MS / input->centralFreq() / 2.0f;
  output->mDl = C_MS / (input->centralFreq() * IMAGE_OUTPUT_SIZE * mDuv);
  output->mSkyMap.setZero();

  // Create Stokes I image
  for (int p = 0; p < NUM_USED_POLARIZATIONS; p++)
  {
    // Zero the grid and regrid
    mGridded.setZero();

    // Set main diagonal to 1 to ensure power > 0
    /*
    const_cast<StreamBlob*>(input)->mCleanData[p].diagonal().setOnes();
    const_cast<StreamBlob*>(input)->mMasks[p].diagonal().setZero();
    for (int i = 0, n = input->mFlagged[p].size(); i < n; i++)
    {
      int dipole = input->mFlagged[p][i];
      const_cast<StreamBlob*>(input)->mMasks[p](dipole, dipole) = 1.0f;
    }
    */

    // Splat the image on a grid
    gridding(input->mCleanData[p], mUCoords, mVCoords, input->mMasks[p], mGridded);

    // Perform fft
    fftShift(mGridded);
    mGridded.reverseInPlace();
    mGridded = mGridded.array().conjugate();
    fftwf_execute(mFFTWPlan);
    fftShift(mGridded);

    // Compute (XX+YY)/2 = I
    output->mSkyMap.array() += mGridded.real().array() * 0.5f;
  }

  // Mask out edges
  float dl = output->mDl*output->mDl;
  output->mSkyMap = (mMask.array() * dl < 1.0f).select(
        mGridded.array().real().transpose(),
        MatrixXf::Zero(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE)
  );
}

void Imager::fftShift(MatrixXcf &matrix)
{
  Q_ASSERT(matrix.rows() == matrix.cols());
  Q_ASSERT(matrix.rows() % 2 == 0);

  int half = matrix.rows() / 2;

  matrix.block(0, 0, half, half).swap(matrix.block(half, half, half, half));
  matrix.block(0, half, half, half).swap(matrix.block(half, 0, half, half));
}

void Imager::gridding(const MatrixXcf &correlations, const MatrixXf &uCoords, const MatrixXf &vCoords, const MatrixXf &mask, MatrixXcf &grid)
{

  Q_ASSERT(correlations.rows() == correlations.cols());

  Q_ASSERT(uCoords.rows() == correlations.rows());
  Q_ASSERT(vCoords.rows() == correlations.rows());
  Q_ASSERT(uCoords.cols() == correlations.cols());
  Q_ASSERT(vCoords.cols() == correlations.cols());

  Q_ASSERT(correlations.rows() == mask.rows());
  Q_ASSERT(correlations.cols() == mask.cols());

  int N = correlations.rows();
  for (int a1 = 0; a1 < N; a1++)
  {
    for (int a2 = 0; a2 < N; a2++)
    {
      if (mask(a1,a2) > 0.5f)
        continue;

      const std::complex<float> &corr = correlations(a1, a2);

      float u = uCoords(a1, a2) / mDuv + IMAGE_OUTPUT_SIZE / 2 - 1;
      float v = vCoords(a1, a2) / mDuv + IMAGE_OUTPUT_SIZE / 2 - 1;

      int w = std::floor(u);
      int e = std::ceil(u);
      int s = std::floor(v);
      int n = std::ceil(v);

      float west_power  = 1.0f - (u - w);
      float east_power  = 1.0f - (e - u);
      float south_power = 1.0f - (v - s);
      float north_power = 1.0f - (n - v);

      float south_west_power = south_power * west_power;
      float north_west_power = north_power * west_power;
      float south_east_power = south_power * east_power;
      float north_east_power = north_power * east_power;

      grid(s, w) += south_west_power * corr;
      grid(n, w) += north_west_power * corr;
      grid(s, e) += south_east_power * corr;
      grid(n, e) += north_east_power * corr;
    }
  }
}
