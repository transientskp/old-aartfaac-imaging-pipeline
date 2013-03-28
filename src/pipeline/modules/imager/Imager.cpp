#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../utilities/AntennaPositions.h"
#include "../../../utilities/Utils.h"
#include "../../../Constants.h"

#include <pelican/utility/Config.h>
#include <QtCore>
#include <limits>

Imager::Imager(const ConfigNode &inConfig):
 AbstractModule(inConfig),
 mDuv(2.5f)
{
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
  // Splat the image on a grid
  gridding(input->mXX, mUCoords, mVCoords, input->mMask, mGridded);

  // Perform fft
  fftShift(mGridded);
  mGridded.reverseInPlace();
  mGridded = mGridded.array().conjugate();
  fftwf_execute(mFFTWPlan);
  fftShift(mGridded);

  // Copy real part to skymap and mask beyond the horizon
  float dl = C_MS / (input->mFrequency * IMAGE_OUTPUT_SIZE * mDuv);
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
  {
    float l = dl*(i-IMAGE_OUTPUT_SIZE/2);
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
    {
      float m = dl*(j-IMAGE_OUTPUT_SIZE/2);
      if (l*l + m*m < 1.0f)
        output->mSkyMap(i,j) = mGridded(i,j).real();
      else
        output->mSkyMap(i,j) = 0.0;
    }
  }
}

void Imager::fftShift(MatrixXcf &ioMatrix)
{
  Q_ASSERT(ioMatrix.rows() == ioMatrix.cols());
  Q_ASSERT(ioMatrix.rows() % 2 == 0);

  int half = ioMatrix.rows() / 2;

  ioMatrix.block(0, 0, half, half).swap(ioMatrix.block(half, half, half, half));
  ioMatrix.block(0, half, half, half).swap(ioMatrix.block(half, 0, half, half));
}

void Imager::gridding(const MatrixXcf &inCorrelations, const MatrixXf &inX, const MatrixXf &inY, const MatrixXf &inMask, MatrixXcf &outGridded)
{

  Q_ASSERT(inCorrelations.rows() == inCorrelations.cols());

  Q_ASSERT(inX.rows() == inCorrelations.rows());
  Q_ASSERT(inY.rows() == inCorrelations.rows());
  Q_ASSERT(inX.cols() == inCorrelations.cols());
  Q_ASSERT(inY.cols() == inCorrelations.cols());

  outGridded.setZero();

  Q_ASSERT(inCorrelations.rows() == inMask.rows());
  Q_ASSERT(inCorrelations.cols() == inMask.cols());

  int N = inCorrelations.rows();
  for (int a1 = 0; a1 < N; a1++)
  {
    for (int a2 = 0; a2 < N; a2++)
    {
      if (inMask(a1,a2) > 0.5f)
        continue;

      const std::complex<float> &corr = inCorrelations(a1, a2);

      float u = inX(a1, a2) / mDuv + IMAGE_OUTPUT_SIZE / 2 - 1;
      float v = inY(a1, a2) / mDuv + IMAGE_OUTPUT_SIZE / 2 - 1;

      int w = std::floor(u);
      int e = std::ceil(u);
      int s = std::floor(v);
      int n = std::ceil(v);

      float west_power  = 1.0f - std::abs(u - w);
      float east_power  = 1.0f - std::abs(u - e);
      float south_power = 1.0f - std::abs(v - s);
      float north_power = 1.0f - std::abs(v - n);

      float south_west_power = south_power * west_power;
      float north_west_power = north_power * west_power;
      float south_east_power = south_power * east_power;
      float north_east_power = north_power * east_power;

      outGridded(s, w) += south_west_power * corr;
      outGridded(n, w) += north_west_power * corr;
      outGridded(s, e) += south_east_power * corr;
      outGridded(n, e) += north_east_power * corr;
    }
  }
}
