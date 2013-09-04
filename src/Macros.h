#ifndef MACROS_H
#define MACROS_H

/** retrieve visiblities from channel 0, xx polarization */
#define Data(o) o->mData[0][XX_POL]

/** retrieve mask from channel 0, xx polarization */
#define Mask(o) o->mMasks[0][XX_POL]

/** retrieve flagged from channel 0, xx polarization */
#define Flagged(o) o->mFlagged[0][XX_POL]

#endif // MACROS_H
