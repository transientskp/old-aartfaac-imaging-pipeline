#ifndef VISIBILITIES_H
#define VISIBILITIES_H

#include <casacore/tables/Tables/TableParse.h>
#include <casacore/ms/MeasurementSets.h>
#include <pelican/output/AbstractOutputStream.h>
#include <pelican/utility/ConfigNode.h>
#include <vector>


using namespace casa;
using namespace pelican;

class UVWParser;

class Visibilities : public AbstractOutputStream
{
public:
  /// Constructor
  Visibilities(const ConfigNode &inConfigNode);

  /// Destructor
  ~Visibilities();

protected:
  /// Send the datablob to the output stream
  void sendStream(const QString &inStreamName, const DataBlob *inDataBlob);

private:
  QString mPath; ///< Path to store visibilities at
  String mTableName; ///< MS input table name
  static std::vector<int> sUpperTriangleIndices;
  static std::vector<Array<Double> > sUVWCoordinates;
  static Array<Float> sWeightSpectrum;
  static double sExposure;
};

PELICAN_DECLARE(AbstractOutputStream, Visibilities)

#endif // VISIBILITIES_H