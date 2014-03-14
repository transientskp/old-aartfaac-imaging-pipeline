#include "AntennaPositions.h"

#include <QtCore>

AntennaPositions *AntennaPositions::sInstance = NULL;

AntennaPositions::AntennaPositions(const QString &filename)
{
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    qFatal("Failed opening %s", qPrintable(filename));

  QTextStream ts(&file);
  mPosItrf.resize(NUM_ANTENNAS, 3);

  QStringList list;
  bool success;
  int idx = 0;
  while (!ts.atEnd())
  {
    QString line = ts.readLine();
    if (line.at(0) == '#' || line.size() == 0)
      continue;

    list = line.split(" ");
    for (int i = 0; i < 3; i++)
    {
      mPosItrf(idx, i) = list.at(i).toDouble(&success);
      Q_ASSERT(success);
    }
    idx++;
  }
  Q_ASSERT(idx == NUM_ANTENNAS);

  // Rotation matrix taken from AntennaField.conf file from CS002
  Matrix3d rot_mat;
  rot_mat << -0.1195950000, -0.7919540000, 0.5987530000,
              0.9928230000, -0.0954190000, 0.0720990000,
              0.0000330000,  0.6030780000, 0.7976820000;

  mPosLocal = mPosItrf * rot_mat;

  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mWCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      mUCoords(a1, a2) = mPosItrf(a1, 0) - mPosItrf(a2, 0);
      mVCoords(a1, a2) = mPosItrf(a1, 1) - mPosItrf(a2, 1);
      mWCoords(a1, a2) = mPosItrf(a1, 2) - mPosItrf(a2, 2);
    }
  }
}

AntennaPositions *AntennaPositions::CreateInstance(const QString &filename)
{
  if (!sInstance)
    sInstance = new AntennaPositions(filename);

  return sInstance;
}

Vector3d AntennaPositions::GetUVW(const int a1, const int a2)
{
  return Vector3d(
    mUCoords(a1, a2),
    mVCoords(a1, a2),
    mWCoords(a1, a2)
  );
}

Vector3d AntennaPositions::GetPosLocal(const int a)
{
  return mPosLocal.row(a);
}

Vector3d AntennaPositions::GetITRF(const int a)
{
  return mPosItrf.row(a);
}
