#include "Visibilities.h"
#include "../../StreamBlob.h"

#include <QtCore>

std::vector<int> Visibilities::sUpperTriangleIndices;
std::vector<Array<Double> > Visibilities::sUVWCoordinates;
Array<Float> Visibilities::sWeightSpectrum;
double Visibilities::sExposure;

Visibilities::Visibilities(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("file", "path", "./");
  mTableName = "../../data/TEST.MS";

  if (sUpperTriangleIndices.empty())
  {
    MeasurementSet ms(mTableName);
    ROMSColumns msc(ms);

    sExposure = msc.exposure()(0);
    sWeightSpectrum = msc.weightSpectrum()(0);
    // From matlab, converts antenna positions to local coords
    const Double rot_m[9] = {
     -0.1195950000, -0.7919540000, 0.5987530000,
      0.9928230000, -0.0954190000, 0.0720990000,
      0.0000330000,  0.6030780000, 0.7976820000
    };

    for (int a1 = 0; a1 < 288; a1++)
    {
      for (int a2 = 0; a2 < 288; a2++)
      {
        if (a1 <= a2)
        {
          sUpperTriangleIndices.push_back(a1 * 288 + a2);

          Array<Double> pos1 = msc.antenna().position()(a1);
          Array<Double> pos2 = msc.antenna().position()(a2);
          Array<Double> uvw(IPosition(1, 3));

          uvw(IPosition(0,0)) = (pos2(IPosition(0,0)) * rot_m[0] +
                                 pos2(IPosition(0,1)) * rot_m[1] +
                                 pos2(IPosition(0,2)) * rot_m[2])
                                -
                                (pos1(IPosition(0,0)) * rot_m[0] +
                                 pos1(IPosition(0,1)) * rot_m[1] +
                                 pos1(IPosition(0,2)) * rot_m[2]);

          uvw(IPosition(0,1)) = (pos2(IPosition(0,0)) * rot_m[3] +
                                 pos2(IPosition(0,1)) * rot_m[4] +
                                 pos2(IPosition(0,2)) * rot_m[5])
                                -
                                (pos1(IPosition(0,0)) * rot_m[3] +
                                 pos1(IPosition(0,1)) * rot_m[4] +
                                 pos1(IPosition(0,2)) * rot_m[5]);

          uvw(IPosition(0,2)) = (pos2(IPosition(0,0)) * rot_m[6] +
                                 pos2(IPosition(0,1)) * rot_m[7] +
                                 pos2(IPosition(0,2)) * rot_m[8])
                                -
                                (pos1(IPosition(0,0)) * rot_m[6] +
                                 pos1(IPosition(0,1)) * rot_m[7] +
                                 pos1(IPosition(0,2)) * rot_m[8]);


          sUVWCoordinates.push_back(uvw);
        }
      }
    }
  }

  Q_ASSERT(sUpperTriangleIndices.size() == (288*289/2));
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
                    QString::number(blob->getFrequency()) +
                    "_" + blob->getDateTime().toString("dd-MM-yyyy_hh-mm-ss") + ".ms");
  String cmd("select from " + mTableName + " where TIME=0 giving " + out_name + " as plain");

  Table table = tableCommand(cmd);
  table.reopenRW();
  MeasurementSet ms(table);
  MSColumns msc(ms);
  Array<Complex> data(IPosition(2, 4, 1));
  Array<Float> ones(IPosition(1, 4));
  ones.set(1.0);

  // The following data table columns hold the default MS values:
  //   FLAG_CATEGORY, ARRAY_ID, DATA_DESC_ID, FEED1, FEED2, FIELD_ID, FLAG_ROW
  //   OBSERVATION_ID, PROCESSOR_ID, SCAN_NUMBER, STATE_ID, FLAG,
  //   LOFAR_FULL_RES_FLAG
  //
  // The others are filled below:
  for (int i = 0, n = sUpperTriangleIndices.size(); i < n; i++)
  {
    ms.addRow();
    int index = sUpperTriangleIndices[i];
    int a1 = (index / 288);
    int a2 = (index % 288);

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
    msc.time().put(i, blob->getMJDTime());
    msc.timeCentroid().put(i, blob->getMJDTime());

    // Update DATA column
    Array<Complex>::iterator it = data.begin();
    (*it).real() = blob->mXXReal[index];
    (*it).imag() = blob->mXXImag[index];
    ++it;
    (*it).real() = blob->mYYReal[index];
    (*it).imag() = blob->mYYImag[index];
    ++it;
    (*it).real() = blob->mXYReal[index];
    (*it).imag() = blob->mXYImag[index];
    ++it;
    (*it).real() = blob->mYXReal[index];
    (*it).imag() = blob->mYXImag[index];
    msc.data().put(i, data);

    // Update WEIGHT_SPECTRUM
    msc.weightSpectrum().put(i, sWeightSpectrum);
  }
  ms.flush();
  ms.closeSubTables();
}

