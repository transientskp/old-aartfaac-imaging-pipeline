#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../utilities/UVWParser.h"
#include "../../../Constants.h"

#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/utility/Config.h>
#include <QtCore>
#include <fstream>
#include <iostream>
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
}

Imager::~Imager()
{
}

void Imager::run(const StreamBlob *input, StreamBlob *output)
{
  Q_UNUSED(output);

  gridding(input->mXX, input->mFlagged);

  std::ofstream real, imag;
  real.open(qPrintable(input->mDateTime.toString("dd-MM-yyyy_hh-mm-ss") + "-R.dat"));
  imag.open(qPrintable(input->mDateTime.toString("dd-MM-yyyy_hh-mm-ss") + "-I.dat"));

  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
  {
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
    {
      float cr = mGridded(i, j).real();
      float ci = mGridded(i, j).imag();
      real << cr << " ";
      imag << ci << " ";
    }
    real << std::endl;
    imag << std::endl;
  }
  real.close();
  imag.close();
}

void Imager::gridding(const MatrixXcf &inCorrelations, const std::vector<int> &inFlagged)
{
  mGridded.setZero();

  static const float sq2 = sqrtf(2.0f);

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    if (inFlagged[a1])
      continue;

    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      if (inFlagged[a2])
        continue;

      const std::complex<float> &corr = inCorrelations(a1, a2);

      float uidx = mUCoords(a1, a2);
      int uidxl = std::floor(uidx);
      int uidxh = std::ceil(uidx);

      Q_ASSERT(uidxl >= 0 && uidxl < IMAGE_OUTPUT_SIZE);
      Q_ASSERT(uidxh >= 0 && uidxh < IMAGE_OUTPUT_SIZE);

      float dul = std::abs(uidx - uidxl);
      float duh = std::abs(uidx - uidxh);

      float vidx = mVCoords(a1, a2);
      int vidxl = std::floor(vidx);
      int vidxh = std::ceil(vidx);

      Q_ASSERT(vidxl >= 0 && vidxl < IMAGE_OUTPUT_SIZE);
      Q_ASSERT(vidxh >= 0 && vidxh < IMAGE_OUTPUT_SIZE);

      float dvl = std::abs(vidx - vidxl);
      float dvh = std::abs(vidx - vidxh);

      float sull = (1.0f - (sqrtf(dul*dul + dvl*dvl) / sq2));
      float suhl = (1.0f - (sqrtf(duh*duh + dvl*dvl) / sq2));
      float sulh = (1.0f - (sqrtf(dul*dul + dvh*dvh) / sq2));
      float suhh = (1.0f - (sqrtf(duh*duh + dvh*dvh) / sq2));

      Q_ASSERT(sull >= 0.0f && sull <= 1.0f);
      Q_ASSERT(suhl >= 0.0f && suhl <= 1.0f);
      Q_ASSERT(sulh >= 0.0f && sulh <= 1.0f);
      Q_ASSERT(suhh >= 0.0f && suhh <= 1.0f);

      mGridded(uidxl, vidxl) += sull * corr;
      mGridded(uidxl, vidxh) += sulh * corr;
      mGridded(uidxh, vidxl) += suhl * corr;
      mGridded(uidxh, vidxh) += suhh * corr;
    }
  }
}









