#include "CasaImageStorage.h"
#include "../../UniboardDataBlob.h"

#include <casacore/images/Images/PagedImage.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <QtCore>

CasaImageStorage::CasaImageStorage(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("file", "path", "./");
}

CasaImageStorage::~CasaImageStorage()
{

}

void CasaImageStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const UniboardDataBlob *blob = static_cast<const UniboardDataBlob*>(inDataBlob);

  if (blob->type() != "UniboardDataBlob")
  {
    qWarning("Expected 'UniboardDataBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

  const std::vector<float> &skymap = blob->getSkyMap();
  QString filename = mPath + "/" +
      QString::number(blob->getFrequency(), 'f', 6) +
      "_" + QString::number(blob->getMJDTime(), 'f', 0) + ".image";

  static casa::TiledShape map_shape(casa::IPosition(2, 512, 512));
  static casa::CoordinateSystem coordinate_info = casa::CoordinateUtil::defaultCoords2D();

  casa::PagedImage<casa::Float> image(map_shape, coordinate_info, qPrintable(filename));
  for (int i = 0, x, y, n = skymap.size(); i < n; i++)
  {
    x = i % 512;
    y = 511 - i / 512;
    image.putAt(skymap[i], casa::IPosition(2, x, y));
  }
}

