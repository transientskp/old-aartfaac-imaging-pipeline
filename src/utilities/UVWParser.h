#ifndef UVWPARSER_H
#define UVWPARSER_H

#include <vector>

#include <QString>

#define MAX_NUM_CHARS 10

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
    UVW(const QString &a1, const QString &a2, const double u, const double v, const double w);
    bool operator <(const UVW &uvw) const;

    int a1;
    int a2;
    int s1;
    int s2;
    char a1_name[MAX_NUM_CHARS];
    char a2_name[MAX_NUM_CHARS];
    double uvw[3];
  };

  UVW GetUVW(const QString &inA1, const QString &inA2, const Type inType);

private:
  static void GetIdAndStation(const QString &inA, int &outId, int &outStation);
  static int GetIndex(const int inA1, const int inS1, const int inA2, const int inS2);

  std::vector<UVW> mUVWPositions;
};

#endif // UVWPARSER_H
