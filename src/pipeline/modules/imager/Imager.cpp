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
  mUCoords.array() *= (IMAGE_OUTPUT_SIZE - 1);

  minv = std::abs<float>(minv);
  mVCoords.array() += minv;
  mVCoords.array() /= minv + maxv;
  mVCoords.array() *= (IMAGE_OUTPUT_SIZE - 1);

  // Create fftw plan
  mFFTWPlan = fftwf_plan_dft_2d(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE,
                               reinterpret_cast<fftwf_complex*>(mGridded.data()),
                               reinterpret_cast<fftwf_complex*>(mGridded.data()),
                               FFTW_FORWARD, FFTW_ESTIMATE|FFTW_PRESERVE_INPUT);

  #ifndef NDEBUG
  qDebug("BEGIN FFTW PLAN");
  fftwf_print_plan(mFFTWPlan);
  qDebug("END FFTW PLAN");
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
  gridding(input->mXX, input->mFlagged);

  // Perform fft
  fftShift();
  fftwf_execute(mFFTWPlan);
  fftShift();

  // Copy real part to skymap and flip over hor and vert axis
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      output->mSkyMap(i,j) = mGridded(i,IMAGE_OUTPUT_SIZE-j-1).real();
}

void Imager::fftShift()
{
  int half = IMAGE_OUTPUT_SIZE/2;
  int q2_i, q2_j, q3_i, q3_j, q4_i, q4_j;
  for (int q1_i = 0; q1_i < IMAGE_OUTPUT_SIZE/2; q1_i++)
  {
    for (int q1_j = 0; q1_j < IMAGE_OUTPUT_SIZE/2; q1_j++)
    {
      q3_i = q1_i + half;
      q3_j = q1_j + half;
      q4_i = q1_i + half;
      q4_j = q1_j;
      q2_i = q1_i;
      q2_j = q1_j + half;
      std::swap(mGridded(q1_i, q1_j), mGridded(q3_i, q3_j));
      std::swap(mGridded(q4_i, q4_j), mGridded(q2_i, q2_j));
    }
  }
}

void Imager::gridding(const MatrixXcf &inCorrelations, const std::vector<int> &inFlagged)
{
  mGridded.setZero();

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    if (inFlagged[a1])
      continue;

    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      if (inFlagged[a2])
        continue;

      const std::complex<float> &corr = inCorrelations(a1, a2);

      float u = mUCoords(a1, a2);
      float v = mVCoords(a1, a2);

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

      mGridded(s, w) += south_west_power * corr;
      mGridded(n, w) += north_west_power * corr;
      mGridded(s, e) += south_east_power * corr;
      mGridded(n, e) += north_east_power * corr;
    }
  }
}
