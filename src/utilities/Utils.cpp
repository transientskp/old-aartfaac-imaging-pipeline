#include "Utils.h"

namespace utils
{
  QDateTime MJD2QDateTime(const double inMJD)
  {
    static const double secs_in_day = 24.0 * 60.0 * 60.0;

    // Convert modified julian date to unix time
    quint32 unix_time = static_cast<quint32>(inMJD + secs_in_day * (2400000.5 - 2440587.5));

    return QDateTime::fromTime_t(unix_time);
  }
}
