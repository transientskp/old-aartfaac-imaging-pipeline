#include "CasaImageStorage.h"
#include "../../UniboardDataBlob.h"

#include <casacore/images/Images/PagedImage.h>
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
  Q_UNUSED(inStreamName);

  const UniboardDataBlob *blob = static_cast<const UniboardDataBlob*>(inDataBlob);

  if (blob->type() != "UniboardDataBlob")
  {
    qWarning("Expected datablob type 'UniboardDataBlob', got '%s' ignoring...", qPrintable(blob->type()));
    return;
  }

  const std::vector<float> &skymap = blob->getSkyMap();
  QString filename = mPath + "/" +
      QString::number(blob->getFrequency(), 'f', 6) +
      "_" + blob->getDateTime().toString("dd-MM-yyyy_hh:mm:ss") + ".image";

  casa::TiledShape map_shape;
  casa::CoordinateSystem coordinate_info;
  casa::PagedImage<casa::Float> image(map_shape, coordinate_info, qPrintable(filename));

  Q_UNUSED(image);
  Q_UNUSED(skymap);
}

