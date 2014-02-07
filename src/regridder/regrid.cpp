#include <casacore/tables/Tables.h>
#include <coordinates/Coordinates.h>
#include <measures/Measures.h>
#include <measures/Measures/MDirection.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageRegrid.h>
#include <scimath/Mathematics/Interpolate2D.h>

#include <iostream>

using namespace casa;

int main(int argc, char *argv[])
{
    (void) argc;
  PagedImage<Float> input_image(argv[1]);
  IPosition dims = input_image.shape();
  const int output_size = dims(0);
  Table &table = input_image.table();
  const TableRecord &record = table.keywordSet();

  Double time = record.asRecord("coords").asRecord("obsdate").asRecord("m0").asDouble("value");
  String unit = record.asRecord("coords").asRecord("obsdate").asRecord("m0").asString("unit");
  Quantum<Double> mjd(time, unit);

  String refer = record.asRecord("coords").asRecord("telescopeposition").asString("refer");
  Double m0 = record.asRecord("coords").asRecord("telescopeposition").asRecord("m0").asDouble("value");
  unit = record.asRecord("coords").asRecord("telescopeposition").asRecord("m0").asString("unit");
  Quantum<Double> M0(m0, unit);
  Double m1 = record.asRecord("coords").asRecord("telescopeposition").asRecord("m1").asDouble("value");
  unit = record.asRecord("coords").asRecord("telescopeposition").asRecord("m1").asString("unit");
  Quantum<Double> M1(m1, unit);
  Double m2 = record.asRecord("coords").asRecord("telescopeposition").asRecord("m2").asDouble("value");
  unit = record.asRecord("coords").asRecord("telescopeposition").asRecord("m2").asString("unit");
  Quantum<Double> M2(m2, unit);

  MEpoch epoch(mjd, MEpoch::Ref(MEpoch::UTC));
  MPosition pos(MVPosition(M2, M0, M1), MPosition::Ref(MPosition::ITRF));
  MeasFrame mf(pos, epoch);
  MDirection mdir = MDirection::Convert(
              MDirection(
                  MVDirection(Quantum<Double>(0.0, "deg"), Quantum<Double>(90.0, "deg")),
                  MDirection::Ref(MDirection::AZEL, mf)
              ),
              MDirection::Ref(MDirection::J2000)
             )();
  Quantum<Vector<Double> > angles = mdir.getAngle();
  Vector<Double> &values = angles.getValue();
  Vector<Double>::iterator i = values.begin();


  casa::Quantum<Double> refLon(*i, angles.getUnit());
  ++i;
  casa::Quantum<Double> refLat(*i, angles.getUnit());

  Double inc = *record.asRecord("coords").asRecord("direction0").asArrayDouble("cdelt").begin();
  unit = *record.asRecord("coords").asRecord("direction0").asArrayString("units").begin();
  casa::Quantum<Double> incLon(inc, unit);
  casa::Quantum<Double> incLat(inc, unit);

  casa::Matrix<Double> xform(2, 2);
  xform = 0.0; xform.diagonal() = 1.0;

  casa::DirectionCoordinate radec(
    casa::MDirection::J2000,
    casa::Projection::SIN,
    refLon, refLat,
    incLon, incLat,
    xform,
    output_size/2, output_size/2
  );

  casa::TiledShape map_shape(casa::IPosition(2, output_size, output_size));
  casa::CoordinateSystem wanted_cs;
  wanted_cs.addCoordinate(radec);

  std::cout << input_image.coordinates().nCoordinates() << std::endl;
  std::cout << wanted_cs.nCoordinates() << std::endl;
  casa::CoordinateSystem output_cs;
  LogIO os;
  output_cs = casa::ImageRegrid<casa::Float>::makeCoordinateSystem(
    os, wanted_cs, input_image.coordinates(), casa::IPosition(2, 0, 1)
  );

  casa::PagedImage<casa::Float> output_image(map_shape, output_cs, "reproj.image");

  ImageRegrid<float> regridder;
  regridder.regrid(output_image, casa::Interpolate2D::CUBIC, casa::IPosition(2, 0, 1), input_image);

  return 0;
}
