#include "CasaImageStorage.h"
#include "../../StreamBlob.h"
#include "../../../Constants.h"

#include <casacore/images/Images/PagedImage.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <QtCore>

CasaImageStorage::CasaImageStorage(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("output", "path", "./");
}

CasaImageStorage::~CasaImageStorage()
{

}

void CasaImageStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const StreamBlob *blob = static_cast<const StreamBlob *>(inDataBlob);

  if (blob->type() != "StreamBlob")
  {
    qWarning("Expected 'StreamBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

  QString filename = mPath + "/" +
                     QString::number(blob->mFrequency) +
                     "_" + blob->mDateTime.toString("dd-MM-yyyy_hh-mm-ss") + ".image";

  static casa::TiledShape map_shape(casa::IPosition(2, IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE));

  static casa::CoordinateSystem coordinate_info = casa::CoordinateUtil::defaultCoords2D();

  casa::PagedImage<casa::Float> image(map_shape, coordinate_info, qPrintable(filename));

  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      image.putAt(blob->mSkyMap(i, j), casa::IPosition(2, i, j));
}

