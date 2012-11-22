#include "UVWParser.h"
#include "../Constants.h"

#include <algorithm>
#include <QtCore>

UVWParser::UVWParser(const QString &inFileName)
{
  QFile file(inFileName);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    qFatal("Failed opening %s", qPrintable(inFileName));

  QTextStream ts(&file);

  int begin, end;
  double u, v, w;
  bool success;
  QString a1, a2, line, tmp;
  QStringList list;
  while (!ts.atEnd())
  {
    line = ts.readLine();

    a1 = line.mid(0, 8); // antenna name 1
    a2 = line.mid(9, 8); // antenna name 2

    begin = line.lastIndexOf('[')+1;
    end = line.lastIndexOf(']')-1;

    tmp = line.mid(begin, end-begin);
    list = tmp.split(", ");
    Q_ASSERT(list.size() == 3);

    u = list.at(0).toDouble(&success);
    Q_ASSERT(success);

    v = list.at(1).toDouble(&success);
    Q_ASSERT(success);

    w = list.at(2).toDouble(&success);
    Q_ASSERT(success);

    mUVWPositions.push_back(UVW(a1, a2, u, v, w));
  }

  Q_ASSERT(mUVWPositions.size() == (NUM_TOTAL_ANTENNAS*(NUM_TOTAL_ANTENNAS-1)));

  // Sort vector so we can query by index
  std::sort(mUVWPositions.begin(), mUVWPositions.end());
  qDebug("UVWPositions(%lu):\n  (%s,%s): <%0.8f %0.8f %0.8f>\n  (%s,%s): <%0.8f %0.8f %0.8f>", 
    mUVWPositions.size(), mUVWPositions.front().a1_name,
    mUVWPositions.front().a2_name, mUVWPositions.front().uvw[0],
    mUVWPositions.front().uvw[1], mUVWPositions.front().uvw[2],
    mUVWPositions.back().a1_name, mUVWPositions.back().a2_name,
    mUVWPositions.back().uvw[0], mUVWPositions.back().uvw[1],
    mUVWPositions.back().uvw[2]);
}

UVWParser::UVW UVWParser::GetUVW(const QString &inA1, const QString &inA2, const Type inType)
{
  Q_ASSERT(!mUVWPositions.empty());

  if (inA1.compare(inA2) == 0)
    return UVW(inA1);

  int a1, s1, a2, s2;
  GetIdAndStation(inA1, a1, s1);
  GetIdAndStation(inA2, a2, s2);

  a1 += inType * (NUM_ANTENNAS_PER_STATION/2);
  a2 += inType * (NUM_ANTENNAS_PER_STATION/2);

  return mUVWPositions[GetIndex(a1, s1, a2, s2)];
}

UVWParser::UVW::UVW():
  a1(-1),
  a2(-1),
  s1(-1),
  s2(-1)
{
  uvw[0] = uvw[1] = uvw[2] = 0.0;
  strcpy(a1_name, "XXXXXXXX");
  strcpy(a2_name, "XXXXXXXX");
}

UVWParser::UVW::UVW(const QString &a)
{
  Q_ASSERT(a.size() <= MAX_NUM_CHARS);

  strcpy(a1_name, qPrintable(a));
  strcpy(a2_name, qPrintable(a));

  UVWParser::GetIdAndStation(a, a1, s1);
  a2 = a1;
  s2 = s1;
  
  uvw[0] = uvw[1] = uvw[2] = 0.0;
}

UVWParser::UVW::UVW(const QString &inA1, const QString &inA2, const double u, const double v, const double w)
{
  Q_ASSERT(inA1.size() <= MAX_NUM_CHARS);
  Q_ASSERT(inA2.size() <= MAX_NUM_CHARS);

  strcpy(a1_name, qPrintable(inA1));
  strcpy(a2_name, qPrintable(inA2));

  UVWParser::GetIdAndStation(inA1, a1, s1);
  UVWParser::GetIdAndStation(inA2, a2, s2);

  uvw[0] = u;
  uvw[1] = v;
  uvw[2] = w;
}

void UVWParser::GetIdAndStation(const QString &inA, int &outId, int &outStation)
{
  QString station = inA.mid(2, 3);
  QString id = inA.mid(6, 2);

  bool success;
  outId = id.toInt(&success);
  Q_ASSERT(success && outId >= 0 && outId < NUM_ANTENNAS_PER_STATION);
  outStation = station.toInt(&success) - 2; // CS002 to CS007
  Q_ASSERT(success && outStation >= 0 && outStation < NUM_STATIONS);
}

inline int UVWParser::GetIndex(const int inA1, const int inS1, const int inA2, const int inS2)
{
  Q_ASSERT(inS1 >= 0 && inS1 < NUM_STATIONS);
  Q_ASSERT(inS2 >= 0 && inS2 < NUM_STATIONS);
  Q_ASSERT(inA1 >= 0 && inA1 < NUM_ANTENNAS_PER_STATION);
  Q_ASSERT(inA2 >= 0 && inA2 < NUM_ANTENNAS_PER_STATION);

  int a1_id = inS1 * NUM_ANTENNAS_PER_STATION + inA1;
  int a2_id = inS2 * NUM_ANTENNAS_PER_STATION + inA2;

  return a1_id * NUM_TOTAL_ANTENNAS + a2_id;
}

bool UVWParser::UVW::operator <(const UVW &uvw) const
{
  int index1 = UVWParser::GetIndex(a1, s1, a2, s2);
  int index2 = UVWParser::GetIndex(uvw.a1, uvw.s1, uvw.a2, uvw.s2);

  return index1 < index2;
}
