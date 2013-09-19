#include "Utils.h"
#include "../Constants.h"

#include <sys/time.h>
#include <stdio.h>

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

void Lock(const char *name)
{
  char filename[1024];
  snprintf(filename, 1024, "%s-%s", LOCK_FILE, name);
  FILE *lock = fopen(filename, "w");
  fclose(lock);
}

bool IsLocked(const char *name)
{
  char filename[1024];
  snprintf(filename, 1024, "%s-%s", LOCK_FILE, name);
  return access(filename, F_OK) != -1;
}

void Unlock(const char *name)
{
  char filename[1024];
  snprintf(filename, 1024, "%s-%s", LOCK_FILE, name);
  remove(filename);
}

void WriteStats(const char *name, const char *data, const char *dir)
{
  char file_name[1024];
  snprintf(file_name, 1024, "%s/%s.dat", dir, name);

  while (IsLocked(name))
    usleep(1e3);

  Lock(name);
    std::ofstream file(file_name, std::ios::out | std::ios::app);
    if (file.is_open())
    {
      file.unsetf(std::ios::floatfield);
      file.precision(30);
      file << data;
      file.close();
    }
    else
    {
      qCritical("Could not open `%s' for writing", file_name);
    }
  Unlock(name);
}

long GetTimeInMicros()
{
  timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec * 1000000 + time.tv_usec;
}
}
