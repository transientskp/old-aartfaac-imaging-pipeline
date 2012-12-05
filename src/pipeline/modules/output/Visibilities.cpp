#include "Visibilities.h"
#include "../../StreamBlob.h"
#include "../../../utilities/UVWParser.h"
#include "../../../Constants.h"

#include <QtCore>

std::vector<int> Visibilities::sUpperTriangleIndices;
std::vector<Array<Double> > Visibilities::sUVWCoordinates;
Array<Float> Visibilities::sWeightSpectrum;
double Visibilities::sExposure;

extern char *gTableName;

Visibilities::Visibilities(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("output", "path", "./");
  QString uvw_file_name = inConfigNode.getOption("uvw", "path");
  UVWParser::Type lba_type = UVWParser::Type(inConfigNode.getOption("lba", "type").toInt());

  UVWParser uvw_parser(uvw_file_name);
  mTableName = gTableName;

  if (sUpperTriangleIndices.empty())
  {
    MeasurementSet ms(mTableName);
    ROMSColumns msc(ms);

    sExposure = msc.exposure()(0);
    sWeightSpectrum = msc.weightSpectrum()(0);

    int a2 = 0;
    while (a2 < NUM_ANTENNAS)
    {
      for (int a1 = 0; a1 < (a2 + 1); a1++)
      {
        Array<Double> uvw(IPosition(1, 3));

        String a1_name = msc.antenna().name()(a1);
        String a2_name = msc.antenna().name()(a2);

        UVWParser::UVW my_uvw = uvw_parser.GetUVW(a1_name.c_str(),
                                                  a2_name.c_str(),
                                                  lba_type);

        uvw(IPosition(1,0)) = Double(my_uvw.uvw[0]);
        uvw(IPosition(1,1)) = Double(my_uvw.uvw[1]);
        uvw(IPosition(1,2)) = Double(my_uvw.uvw[2]);

        sUVWCoordinates.push_back(uvw);
        sUpperTriangleIndices.push_back(a1 * NUM_ANTENNAS + a2);
      }

      a2++;
    }
  }

  Q_ASSERT(sUpperTriangleIndices.size() == (NUM_ANTENNAS*(NUM_ANTENNAS+1))/2);
}

Visibilities::~Visibilities()
{
}

void Visibilities::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const StreamBlob *blob = static_cast<const StreamBlob *>(inDataBlob);

  if (blob->type() != "StreamBlob")
  {
    qWarning("Expected 'StreamBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

  String out_name = qPrintable(mPath + "/" +
                    QString::number(blob->mFrequency) +
                    "_" + blob->mDateTime.toString("dd-MM-yyyy_hh-mm-ss") + ".ms");
  String cmd("select from " + mTableName + " where TIME=0 giving " + out_name + " as plain");

  Table table = tableCommand(cmd);
  table.reopenRW();
  MeasurementSet ms(table);
  MSColumns msc(ms);
  Array<Complex> data(IPosition(2, 4, 1));
  Array<Bool> flags(IPosition(2, 4, 1));
  Array<Float> ones(IPosition(1, 4));
  ones.set(1.0);

  // The following data table columns hold the default MS values:
  //   FLAG_CATEGORY, ARRAY_ID, DATA_DESC_ID, FEED1, FEED2, FIELD_ID, FLAG_ROW
  //   OBSERVATION_ID, PROCESSOR_ID, SCAN_NUMBER, STATE_ID, LOFAR_FULL_RES_FLAG
  //
  // The others are filled below:
  for (int i = 0, n = sUpperTriangleIndices.size(); i < n; i++)
  {
    ms.addRow();
    int index = sUpperTriangleIndices[i];
    int a1 = (index / NUM_ANTENNAS);
    int a2 = (index % NUM_ANTENNAS);

    // Update UVW
    msc.uvw().put(i, sUVWCoordinates[i]);

    // Update WEIGHT
    msc.weight().put(i, ones);

    // Update SIGMA
    msc.sigma().put(i, ones);

    // Update ANTENNA1 and ANTENNA2
    msc.antenna1().put(i, a1);
    msc.antenna2().put(i, a2);

    // Update EXPOSURE and INTERVAL
    msc.exposure().put(i, sExposure);
    msc.interval().put(i, sExposure);

    // Update TIME and TIME_CENTROID
    msc.time().put(i, blob->mMJDTime);
    msc.timeCentroid().put(i, blob->mMJDTime);

    // Update DATA column
    data(IPosition(2, 0, 0)) = blob->mXX(a1, a2);
    data(IPosition(2, 1, 0)) = blob->mYY(a1, a2);
    data(IPosition(2, 2, 0)) = blob->mXY(a1, a2);
    data(IPosition(2, 3, 0)) = blob->mYX(a1, a2);
    msc.data().put(i, data);

    // Update FLAG column
    // TODO: Perform flagging per polarization
    flags(IPosition(2, 0, 0)) = false;
    flags(IPosition(2, 1, 0)) = false;
    flags(IPosition(2, 2, 0)) = false;
    flags(IPosition(2, 3, 0)) = false;
    if (blob->mFlagged[a1] || blob->mFlagged[a2])
    {
      flags(IPosition(2, 0, 0)) = true;
      flags(IPosition(2, 1, 0)) = true;
      flags(IPosition(2, 2, 0)) = true;
      flags(IPosition(2, 3, 0)) = true;
    }
    msc.flag().put(i, flags);

    // Update WEIGHT_SPECTRUM
    msc.weightSpectrum().put(i, sWeightSpectrum);
  }

  ms.flush();
  ms.closeSubTables();
}
