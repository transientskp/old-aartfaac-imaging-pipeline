#ifndef CONSTANTS_H
#define CONSTANTS_H

/** Max number of chars for the antenna names */
#define MAX_CHARS_ANTENNA_NAME 10

/** Number of active antennas */
#define NUM_ANTENNAS 288

/** Number of antennas per station */
#define NUM_ANTENNAS_PER_STATION 96

/** Number of stations */
#define NUM_STATIONS 6

/** Number of total antennas */
#define NUM_TOTAL_ANTENNAS (NUM_ANTENNAS_PER_STATION*NUM_STATIONS)

/** Skymap image resolution */
#define IMAGE_OUTPUT_SIZE 512

#endif // CONSTANTS_H
