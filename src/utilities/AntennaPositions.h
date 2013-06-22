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

  MatrixXd& GetAllU() { return mUCoords; }
  MatrixXd& GetAllV() { return mVCoords; }
  MatrixXd& GetAllW() { return mWCoords; }

  MatrixXd& GetAllITRF() { return mAntennaITRF; }

private:
  MatrixXd mAntennaITRF; ///< ITRF positions (a1,...,an)
  MatrixXd mUCoords;     ///< a1_x - a2_x;
  MatrixXd mVCoords;     ///< a1_y - a2_y;
  MatrixXd mWCoords;     ///< a1_z - a2_z;
};

static AntennaPositions ap("../data/pos_local.dat");

#define ANT_UVW(a, b) ap.GetUVW(a, b)
#define ANT_U() ap.GetAllU()
#define ANT_V() ap.GetAllV()
#define ANT_W() ap.GetAllW()
#define ANT_ITRF() ap.GetAllITRF()
#define ANT_XYZ(a) ap.GetITRF(a)

#endif // ANTENNA_POSITONS_H
