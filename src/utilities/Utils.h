#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>

namespace utils
{
/**
 * @brief
 * Converts a modified julian date in seconds to QDateTime
 */
QDateTime MJD2QDateTime(const double inMJD);

/**
 * @brief
 * Get the current system time in milliseconds
 */
long GetTimeInMicros();
}

#endif // UTILITIES_H
