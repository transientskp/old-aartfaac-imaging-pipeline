#include <casacore/tables/Tables.h>
#include <coordinates/Coordinates.h>
#include <measures/Measures.h>
#include <measures/Measures/MDirection.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageRegrid.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <iostream>
#include <string>

using namespace casa;

void print_help()
{
  printf("Usage: regridder in=/path/to/input.image out=/path/to/output.image\n");
}

std::string extract_path(const std::string &s)
{
  std::size_t pos = s.find_first_of('=');
  if (pos > s.size())
    exit(1);

  return s.substr(pos+1);
}

int main(int argc, char *argv[])
{
  std::string infile("");
  std::string outfile("");
  switch(argc)
  {
    case 2:
      print_help();
      return 0;
    case 3:
    {
      std::string tmp(argv[1]);
      if (tmp.compare(0, 2, "in") == 0)
        infile = extract_path(argv[1]);
      tmp = argv[2];
      if (tmp.compare(0, 3, "out") == 0)
        outfile = extract_path(argv[2]);
      break;
    }
    default:
      print_help();
      return 1;
  }

  if (infile.empty() || outfile.empty())
  {
    print_help();
    return 1;
  }

  PagedImage<Float> input_image(infile.c_str());
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

  casa::TiledShape map_shape(dims);
  casa::CoordinateSystem wanted_cs;
  wanted_cs.addCoordinate(radec);

  casa::CoordinateSystem output_cs;
  LogIO os;
  std::set<casa::Coordinate::Type> set;
  output_cs = casa::ImageRegrid<casa::Float>::makeCoordinateSystem(
    os, set, wanted_cs, input_image.coordinates(), casa::IPosition(2, 0, 1)
  );

  casa::PagedImage<casa::Float> output_image(map_shape, output_cs, outfile.c_str());

  ImageRegrid<float> regridder;
  regridder.regrid(output_image, casa::Interpolate2D::CUBIC, casa::IPosition(2, 0, 1), input_image);
  casa::ImageInfo image_info;
  image_info.setRestoringBeam(casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(1.0, "deg"), casa::Quantum<Double>(0.0, "deg"));
  image_info.setImageType(casa::ImageInfo::Intensity);
  image_info.setObjectName("Aartfaac image");
  output_image.setImageInfo(image_info);
  return 0;
}
