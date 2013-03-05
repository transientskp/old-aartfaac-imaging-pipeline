#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../utilities/UVWParser.h"
#include "../../../Constants.h"

#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/utility/Config.h>
#include <QtCore>
#include <limits>

extern char *gTableName;

Imager::Imager(const ConfigNode &inConfig):
 AbstractModule(inConfig)
{
  mGridded.resize(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE);

  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mUCoords.setZero();
  mVCoords.setZero();

  QString uvw_file_name = inConfig.getOption("uvw", "path");
  UVWParser::Type lba_type = UVWParser::Type(inConfig.getOption("lba", "type").toInt());

  UVWParser uvw_parser(uvw_file_name);

  casa::MeasurementSet ms(gTableName);
  casa::ROMSColumns msc(ms);

  float minu = std::numeric_limits<float>::max(), maxu = std::numeric_limits<float>::min();
  float minv = std::numeric_limits<float>::max(), maxv = std::numeric_limits<float>::min();

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

      minu = std::min<float>(minu, mUCoords(a1,a2));
      minv = std::min<float>(minv, mVCoords(a1,a2));
      maxu = std::max<float>(maxu, mUCoords(a1,a2));
      maxv = std::max<float>(maxv, mVCoords(a1,a2));
    }
  }


  minu = std::abs<float>(minu);
  mUCoords.array() += minu;
  mUCoords.array() /= minu + maxu;
  mUCoords.array() *= (UV_GRID_SIZE - 1);

  minv = std::abs<float>(minv);
  mVCoords.array() += minv;
  mVCoords.array() /= minv + maxv;
  mVCoords.array() *= (UV_GRID_SIZE - 1);

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
  fftwf_execute(mFFTWPlan);
  fftShift(mGridded);

  // Copy real part to skymap and flip over vert axis
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      output->mSkyMap(i,j) = mGridded(i,IMAGE_OUTPUT_SIZE-j-1).real();
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

      float u = inX(a1, a2) + IMAGE_OUTPUT_SIZE/2 - UV_GRID_SIZE/2;
      float v = inY(a1, a2) + IMAGE_OUTPUT_SIZE/2 - UV_GRID_SIZE/2;

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
