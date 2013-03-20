#ifndef ANTENNA_POSITONS_H
#define ANTENNA_POSITONS_H

#include <eigen3/Eigen/Dense>
#include <QString>

#include "../Constants.h"

using namespace Eigen;

class AntennaPositions
{
public:
  AntennaPositions(const QString &inFileName);
  ~AntennaPositions() {}

  Vector3d GetITRF(const int a);
  Vector3d GetUVW(const int a1, const int a2);

private:
  MatrixXd mAntennaITRF; ///< ITRF positions (a1,...,an)
  MatrixXd mUCoords;     ///< a1_x - a2_x;
  MatrixXd mVCoords;     ///< a1_y - a2_y;
  MatrixXd mWCoords;     ///< a1_z - a2_z;
};

static const AntennaPositions ap("../data/posITRF.dat");

#define UVW(a, b) ap.GetUVW(a, b)
#define ITRF(a) ap.GetITRF(a)

#endif // ANTENNA_POSITONS_H
