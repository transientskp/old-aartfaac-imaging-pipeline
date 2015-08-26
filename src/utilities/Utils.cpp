#include "Utils.h"
#include "Constants.h"

#include <sys/time.h>
#include <sys/file.h>
#include <stdio.h>

namespace utils
{
static const double seconds_between_julian_and_unix_epoch = 3506716800.0;

QDateTime MJD2QDateTime(const double inMJDsec)
{
  // Convert modified julian date to unix time
  quint32 unix_time = static_cast<quint32>(MJD2UnixTime(inMJDsec));

  return QDateTime::fromTime_t(unix_time).toUTC();
}

double MJD2UnixTime(const double inMJD)
{
  return inMJD - seconds_between_julian_and_unix_epoch;
}

double UnixTime2MJD(const double inUnixTime)
{
  return inUnixTime + seconds_between_julian_and_unix_epoch;
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

void WriteStats(const char *name, const char *data, const char *dir)
{
  char file_name[1024];
  snprintf(file_name, 1024, "%s/%s.dat", dir, name);

  FILE *fp = fopen(file_name, "a+");
  if (fp != NULL)
  {
    int fn = fileno(fp);
    flock(fn, LOCK_EX);
    fputs(data, fp);
    fclose(fp);
    flock(fn, LOCK_UN);
  }
  else
  {
    qCritical("Could not open `%s' for writing", file_name);
  }
}

long GetTimeInMicros()
{
  timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000000 + time.tv_usec;
}
}
