#include "CasaImageStorage.h"
#include "../../StreamBlob.h"
#include "../../../Constants.h"
#include "../../../utilities/Utils.h"

#include <casacore/images/Images/PagedImage.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates.h>
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

  QString filename = mPath + "/F" + QString::number(blob->mHeader.freq) + "_S" +
                     QString::number(blob->mHeader.start_chan) + "-" +
                     QString::number(blob->mHeader.end_chan) + "_T" +
                     utils::MJD2QDateTime(blob->mHeader.time).toString("dd-MM-yyyy_hh-mm-ss") +
                     ".image";

  static casa::Matrix<Double> xform(2,2);
  xform = 0.0; xform.diagonal() = 1.0;
  static casa::Quantum<Double> refLon(0, "deg");
  static casa::Quantum<Double> refLat(90, "deg");
  static casa::Quantum<Double> incLon(-2.5e-2, "deg");
  static casa::Quantum<Double> incLat(2.5e-2, "deg");
  static casa::DirectionCoordinate azel(MDirection::AZEL,
                            casa::Projection::SIN,
                            refLon, refLat,
                            incLon, incLat,
                            xform,
                            IMAGE_OUTPUT_SIZE/2, IMAGE_OUTPUT_SIZE/2);

  static casa::TiledShape map_shape(casa::IPosition(2, IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE));
  casa::CoordinateSystem coordinate_info; // = casa::CoordinateUtil::defaultCoords2D();
  coordinate_info.addCoordinate(azel);

  casa::PagedImage<casa::Float> image(map_shape, coordinate_info, qPrintable(filename));
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      image.putAt(blob->mSkyMap(i, j), casa::IPosition(2, j, i));
}

