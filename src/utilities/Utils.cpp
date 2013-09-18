#include "Utils.h"
#include <sys/time.h>

namespace utils
{
QDateTime MJD2QDateTime(const double inMJD)
{
  static const double secs_in_day = 24.0 * 60.0 * 60.0;

  // Convert modified julian date to unix time
  quint32 unix_time = static_cast<quint32>(inMJD + secs_in_day * (2400000.5 - 2440587.5));

  return QDateTime::fromTime_t(unix_time);
}

void sunRaDec(const double inJD, double &outRa, double &outDec)
{
  double n = inJD - 2451545.0;
  double L = std::fmod(280.460 + 0.9856474 * n, 360.0);
  double g = std::fmod(357.528 + 0.9856003 * n, 360.0);
  double lambda = L + 1.915 * sin(DEG(g)) + 0.020 * sin(DEG(2*g));
  double epsilon = 23.439 - 0.0000004 * n;
  outRa = atan2(cos(DEG(epsilon)) * sin(DEG(lambda)), cos(DEG(lambda)));
  outDec = asin(sin(DEG(epsilon)) * sin(DEG(lambda)));
}

long GetTimeInMicros()
{
  timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000000 + time.tv_usec;
}
}
