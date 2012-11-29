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
    for (int a2 = a1; a2 < NUM_ANTENNAS; a2++)
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

  gridding(input->mXX);
}

void Imager::gridding(const MatrixXcf &inCorrelations)
{
  mGridded.setZero();

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = a1; a2 < NUM_ANTENNAS; a2++)
    {
      const std::complex<float> &corr = inCorrelations(a1, a2);
      float amplitude = std::abs(corr);
      std::complex<float> phasor = corr / amplitude;

      float uidx = mUCoords(a1, a2);
      int uidxl = std::floor(uidx);
      int uidxh = std::ceil(uidx);
      Q_ASSERT(uidxl >= 0 && uidxl < IMAGE_OUTPUT_SIZE);
      Q_ASSERT(uidxh >= 0 && uidxh < IMAGE_OUTPUT_SIZE);

      float dul = std::abs(uidx - uidxl);
      float duh = std::abs(uidx - uidxh);

      float sul = duh * amplitude;
      float suh = dul * amplitude;

      float vidx = mVCoords(a1, a2);
      int vidxl = std::floor(vidx);
      int vidxh = std::ceil(vidx);
      Q_ASSERT(vidxl >= 0 && vidxl < IMAGE_OUTPUT_SIZE);
      Q_ASSERT(vidxh >= 0 && vidxh < IMAGE_OUTPUT_SIZE);

      float dvl = std::abs(vidx - vidxl);
      float dvh = std::abs(vidx - vidxh);

      float sull = dvh * sul;
      float suhl = dvh * suh;
      float sulh = dvl * sul;
      float suhh = dvl * suh;

      // Update upper triangle, include diagonal
      mGridded(uidxl, vidxl) += sull * phasor;
      mGridded(uidxl, vidxh) += sulh * phasor;
      mGridded(uidxh, vidxl) += suhl * phasor;
      mGridded(uidxh, vidxh) += suhh * phasor;

      // Update lower triangle, exclude diagonal
      if (a1 != a2)
      {
        mGridded(vidxl, uidxl) += sull * phasor;
        mGridded(vidxl, uidxh) += sulh * phasor;
        mGridded(vidxh, uidxl) += suhl * phasor;
        mGridded(vidxh, uidxh) += suhh * phasor;
      }
    }
  }
}









