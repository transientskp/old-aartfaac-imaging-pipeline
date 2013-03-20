#include "AntennaPositions.h"

#include <QtCore>

AntennaPositions::AntennaPositions(const QString &inFileName)
{
  QFile file(inFileName);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    qFatal("Failed opening %s", qPrintable(inFileName));

  QTextStream ts(&file);
  mAntennaITRF.resize(NUM_ANTENNAS, 3);

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
      mAntennaITRF(idx, i) = list.at(i).toDouble(&success);
      Q_ASSERT(success);
    }
    idx++;
  }
  Q_ASSERT(idx == NUM_ANTENNAS);

  mUCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mVCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);
  mWCoords.resize(NUM_ANTENNAS, NUM_ANTENNAS);

  for (int a1 = 0; a1 < NUM_ANTENNAS; a1++)
  {
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      mUCoords(a1, a2) = mAntennaITRF(a1, 0) - mAntennaITRF(a2, 0);
      mVCoords(a1, a2) = mAntennaITRF(a1, 1) - mAntennaITRF(a2, 1);
      mWCoords(a1, a2) = mAntennaITRF(a1, 2) - mAntennaITRF(a2, 2);
    }
  }
}

Vector3d AntennaPositions::GetUVW(const int a1, const int a2)
{
  return Vector3d(
    mUCoords(a1, a2), 
    mVCoords(a1, a2), 
    mWCoords(a1, a2)
  );
}

Vector3d AntennaPositions::GetITRF(const int a)
{
  return mAntennaITRF.row(a);
}
