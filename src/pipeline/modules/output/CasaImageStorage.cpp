#include "CasaImageStorage.h"
#include "../../StreamBlob.h"
#include "Constants.h"
#include "../../../utilities/Utils.h"

#include <casacore/images/Images/PagedImage.h>
#include <casacore/images/Images/ImageInfo.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates.h>
#include <casacore/measures/Measures/Measure.h>
#include <casacore/measures/Measures/MEpoch.h>
#include <casacore/measures/Measures/MCEpoch.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/casa/Quanta.h>
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

  casa::Matrix<Double> xform(2,2);
  xform = 0.0; xform.diagonal() = 1.0;
  double pixel_dist = asin(blob->mDl * (180.0/M_PI));
  casa::Quantum<Double> refLon(270, "deg");
  casa::Quantum<Double> refLat(90, "deg");
  casa::Quantum<Double> incLon(pixel_dist, "deg");
  casa::Quantum<Double> incLat(pixel_dist, "deg");
  casa::DirectionCoordinate azel(MDirection::AZEL,
                            casa::Projection::SIN,
                            refLon, refLat,
                            incLon, incLat,
                            xform,
                            IMAGE_OUTPUT_SIZE/2, IMAGE_OUTPUT_SIZE/2);

  casa::TiledShape map_shape(casa::IPosition(4, IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE, 1, 1));
  casa::CoordinateSystem coordinate_system;
  coordinate_system.addCoordinate(azel);
  casa::SpectralCoordinate spectral(casa::MFrequency::LSRK, blob->centralFreq(), blob->mNumChannels*blob->mHeader.chan_width, blob->centralFreq());
  spectral.setRestFrequency(blob->centralFreq());
  coordinate_system.addCoordinate(spectral);
  casa::ObsInfo obs_info;
  
  obs_info.setObserver("AARTFAAC Project");
  obs_info.setTelescope("AARTFAAC");
  obs_info.setObsDate(MEpoch(MVEpoch(Quantity(blob->mHeader.time, "s")), MEpoch::Ref(MEpoch::UTC)));
  MPosition obs(MVPosition(3826577.066110000, 461022.947639000, 5064892.786), casa::MPosition::ITRF);
  obs_info.setTelescopePosition(obs);
  MEpoch model(Quantity(0., "d"));
  MeasFrame frame(obs);
// Set up the output reference
  MEpoch::Ref outref(MEpoch::LAST,
                     frame);
  MEpoch::Convert toLST(model,
                        outref);
  MVDirection dir;
  // RA/DEC
  Double ra = toLST(utils::MJD2QDateTime(blob->mHeader.time).toTime_t()).getValue().getDayFraction();
  Double dec = 0.92354311057856478750;
  dir.setAngle(ra*12.0/M_PI, dec);

  obs_info.setPointingCenter(dir);
  coordinate_system.setObsInfo(obs_info);
  casa::Vector<Int> stokes(1);
  stokes(0) = casa::Stokes::I;
  coordinate_system.addCoordinate(casa::StokesCoordinate(stokes));

  casa::ImageInfo image_info;
  image_info.setRestoringBeam(casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(0.0, "deg"));
  image_info.setImageType(casa::ImageInfo::Intensity);
  image_info.setObjectName("Aartfaac image");
  casa::PagedImage<casa::Float> image(map_shape, coordinate_system, qPrintable(filename));
  if (!image.setImageInfo(image_info))
    qCritical("Unable to set image info!");
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      image.putAt(blob->mSkyMap(j, i), casa::IPosition(4, j, i, 0, 0));
}

