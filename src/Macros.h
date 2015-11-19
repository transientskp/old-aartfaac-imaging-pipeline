#ifndef MACROS_H
#define MACROS_H

/** retrieve visiblities from channel 0, xx polarization */
#define Data(o) o->mCleanData[0]

/** retrieve mask from channel 0, xx polarization */
#define Mask(o) o->mMasks[0]

/** retrieve flagged from channel 0, xx polarization */
#define Flagged(o) o->mFlagged[0]

#endif // MACROS_H
