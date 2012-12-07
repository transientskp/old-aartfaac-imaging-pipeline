#ifndef UVWPARSER_H
#define UVWPARSER_H

#include <vector>
#include <QString>

#include "../Constants.h"

class UVWParser
{
public:
  // NOTE: Order is important!
  enum Type{LBA_INNER = 0, LBA_OUTER = 1};

  UVWParser(const QString &inFileName);
  ~UVWParser() {}

  struct UVW
  {
    UVW();
    UVW(const QString &a);
    UVW(const QString &a1, const QString &a2, const float u, const float v, const float w);
    bool operator <(const UVW &uvw) const;

    int a1;
    int a2;
    int s1;
    int s2;
    char a1_name[MAX_CHARS_ANTENNA_NAME];
    char a2_name[MAX_CHARS_ANTENNA_NAME];
    float uvw[3];
  };

  UVW GetUVW(const QString &inA1, const QString &inA2, const Type inType);

private:
  static void GetIdAndStation(const QString &inA, int &outId, int &outStation);
  static int GetIndex(const int inA1, const int inS1, const int inA2, const int inS2);
  static std::vector<UVW> sUVWPositions;
};

#endif // UVWPARSER_H
