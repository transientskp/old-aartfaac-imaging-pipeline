
#include "CasaImageStorage.h"
#include "../../StreamBlob.h"
#include "Constants.h"
#include "../../../utilities/Utils.h"

#include <casacore/images/Images/PagedImage.h>
#include <casacore/images/Images/ImageInfo.h>
#include <casacore/images/Regions/RegionHandler.h>
#include <casacore/coordinates/Coordinates/CoordinateUtil.h>
#include <casacore/coordinates/Coordinates.h>
#include <casacore/measures/Measures/Measure.h>
#include <casacore/measures/Measures/MEpoch.h>
#include <casacore/measures/Measures/MCEpoch.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/casa/Quanta.h>
#include <casacore/lattices/Lattices/LCPagedMask.h>

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

  // Telescope location
  MPosition obs(MVPosition(3826577.066110000, 461022.947639000, 5064892.786), casa::MPosition::ITRF);

  // Determine the RA/Dec of the center of the image, based on the time of obs.
  MEpoch model(Quantity(0., "d")); // Setup model, MEpoch unit of time is MJD, due to the "d".
  MeasFrame frame(obs);

  // Set up the output reference
  MEpoch::Ref outref(MEpoch::LAST, frame); // LAST applies nutation of mean pole from mean to true position
  MEpoch::Convert toLST(model, outref);
  MVDirection dir;

  // RA/DEC in radians (default for setAngle).
  Double ra = toLST((blob->mHeader.time)/86400.).getValue().getDayFraction()*2*M_PI;
  Double dec = 0.92354311057856478750; // Always fixed due to transit mode of AARTFAAC.
  dir.setAngle(ra, dec);
  // Vector<Double> tmp = dir.get();
  // fprintf (stderr, "<-- Setting pointing direction at time %f to RA:%.5f, Dec: %.5f, read: %.5f, %.5f rad.\n", blob->mHeader.time+7200,ra, dec, tmp[0], tmp[1]); 

  casa::Matrix<Double> xform(2,2);
  xform = 0.0; xform.diagonal() = 1.0;
  double pixel_dist = asin(blob->mDl) * (180.0/M_PI);

  // Set the CRVAL to point to the zenith
  casa::Quantum<Double> refLon(ra, "rad");
  casa::Quantum<Double> refLat(dec, "rad");
  casa::Quantum<Double> incLon(-pixel_dist, "deg");
  casa::Quantum<Double> incLat(pixel_dist, "deg");

  // Create a coordinate system for this image
  casa::CoordinateSystem coordinate_system;

  // Specify coordinate system directly as RADEC-J2000
  casa::DirectionCoordinate radec(casa::MDirection::J2000,
						    casa::Projection::SIN,
						    refLon, refLat,
						    incLon, incLat,
						    xform,
                IMAGE_OUTPUT_SIZE/2, IMAGE_OUTPUT_SIZE/2);

  // Create a coordinate system for specifying frequency information of an image.
  casa::SpectralCoordinate spectral(casa::MFrequency::LSRK, blob->centralFreq(), blob->mNumChannels*blob->mHeader.chan_width, 0);// blob->centralFreq());
  spectral.setRestFrequency(blob->centralFreq());

  casa::TiledShape map_shape(casa::IPosition(4, IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE, 1, 1));

  casa::ObsInfo obs_info;
  obs_info.setObserver("AARTFAAC Project");
  obs_info.setTelescope("AARTFAAC");
  obs_info.setObsDate(MEpoch(MVEpoch(Quantity(blob->mHeader.time, "s")), MEpoch::Ref(MEpoch::UTC)));
  obs_info.setTelescopePosition(obs);
  obs_info.setPointingCenter(dir);

  // Add the spatial and spectral frequency information into the coordinate system.
  coordinate_system.addCoordinate(radec);

  coordinate_system.setObsInfo(obs_info);

  casa::Vector<Int> stokes(1);
  stokes(0) = casa::Stokes::I;
  coordinate_system.addCoordinate(spectral);
  coordinate_system.addCoordinate(casa::StokesCoordinate(stokes));

  casa::ImageInfo image_info;
  image_info.setRestoringBeam(casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(0.0, "deg"));
  image_info.setImageType(casa::ImageInfo::Intensity);
  image_info.setObjectName("Aartfaac image");
  casa::PagedImage<casa::Float> image(map_shape, coordinate_system, qPrintable(filename));

  /* Mask generation: Doesn't seem useful, if used, should move to constructor.
  casa::LCPagedMask mask (map_shape, "mask0");
  mask.set( True );
  image.defineRegion ("mask0", mask, casa::RegionHandler::Masks);
  image.setDefaultMask ("mask0");
  */

  if (!image.setImageInfo(image_info))
    qCritical("Unable to set image info!");
  for (int i = 0; i < IMAGE_OUTPUT_SIZE; i++)
    for (int j = 0; j < IMAGE_OUTPUT_SIZE; j++)
      image.putAt(blob->mSkyMap(j, i), casa::IPosition(4, j, (IMAGE_OUTPUT_SIZE-1-i), 0, 0));
}

