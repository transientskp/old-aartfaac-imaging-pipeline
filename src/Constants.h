#ifndef CONSTANTS_H
#define CONSTANTS_H

/** Max number of chars for the antenna names */
#define MAX_CHARS_ANTENNA_NAME 10

/** Number of active antennas */
#define NUM_ANTENNAS 288

/** Number of baselines including self correlation */
#define NUM_BASELINES (NUM_ANTENNAS*(NUM_ANTENNAS+1)/2)

/** Number of channels */
#define NUM_CHANNELS 64

/** Number of polarizations (X, Y) */
#define NUM_POLARIZATIONS 2

/** Polarization indexing for CORR_TYPE 9,10,11,12 */
#define XX_POL 0
#define YY_POL 1
#define XY_POL 2
#define YX_POL 3

/** Number of antennas per station */
#define NUM_ANTENNAS_PER_STATION 96

/** Number of stations */
#define NUM_STATIONS 6

/** Number of total antennas */
#define NUM_TOTAL_ANTENNAS (NUM_ANTENNAS_PER_STATION*NUM_STATIONS)

/** Skymap image resolution */
#define IMAGE_OUTPUT_SIZE 1024

/** Speed of light in meters per second */
#define C_MS 299792458.0

#endif // CONSTANTS_H
