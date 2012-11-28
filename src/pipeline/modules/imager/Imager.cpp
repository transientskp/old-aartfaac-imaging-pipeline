#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../utilities/UVWParser.h"
#include "../../../Constants.h"

#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/utility/Config.h>
#include <QtCore>

extern char *gTableName;

Imager::Imager(const ConfigNode &inConfig):
 AbstractModule(inConfig),
 mGridSize(600.0f/256.0f),
 mGridPoints(500),
 mResolution(IMAGE_OUTPUT_SIZE)
{
  mGridded.resize(mGridPoints, mGridPoints);
  mGridded.setZero();

  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mWCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  QString uvw_file_name = inConfig.getOption("uvw", "path");
  UVWParser::Type lba_type = UVWParser::Type(inConfig.getOption("lba", "type").toInt());

  UVWParser uvw_parser(uvw_file_name);

  casa::MeasurementSet ms(gTableName);
  casa::ROMSColumns msc(ms);

  int a2 = 0;
  while (a2 < NUM_ANTENNAS)
  {
    for (int a1 = 0; a1 < (a2 + 1); a1++)
    {
      casa::String a1_name = msc.antenna().name()(a1);
      casa::String a2_name = msc.antenna().name()(a2);

      UVWParser::UVW uvw = uvw_parser.GetUVW(a1_name.c_str(),
                                             a2_name.c_str(),
                                             lba_type);

      mUCoords(a1, a2) = uvw.uvw[0];
      mVCoords(a1, a2) = uvw.uvw[1];
      mWCoords(a1, a2) = uvw.uvw[2];
    }

    a2++;
  }
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
  Q_UNUSED(inCorrelations);
/*
  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      const std::complex<float> &corr = inCorrelations(a1, a2);
      float amplitude = std::abs(corr);
      std::complex<float> phasor = corr / amplitude;

      float uidx = mUCoords(a1, a2) / mGridSize + mGridPoints / 2.0f;
      int uidxl = std::floor(uidx);
      int uidxh = std::ceil(uidx);
      Q_ASSERT(uidxl >= 0 && uidxl < mGridPoints);
      Q_ASSERT(uidxh >= 0 && uidxh < mGridPoints);

      float dul = std::abs(uidx - uidxl);
      float duh = std::abs(uidx - uidxh);

      float sul = duh * amplitude;
      float suh = dul * amplitude;

      float vidx = mVCoords(a1, a2) / mGridSize + mGridPoints / 2.0f;
      int vidxl = std::floor(vidx);
      int vidxh = std::ceil(vidx);
      Q_ASSERT(vidxl >= 0 && vidxl < mGridPoints);
      Q_ASSERT(vidxh >= 0 && vidxh < mGridPoints);

      float dvl = std::abs(vidx - vidxl);
      float dvh = std::abs(vidx - vidxh);

      float sull = dvh * sul;
      float suhl = dvh * suh;
      float sulh = dvl * sul;
      float suhh = dvl * suh;

      mGridded(uidxl, vidxl) += sull * phasor;
      mGridded(uidxl, vidxh) += sulh * phasor;
      mGridded(uidxh, vidxl) += suhl * phasor;
      mGridded(uidxh, vidxh) += suhh * phasor;
    }
  }
*/
}









