#ifndef MATLAB_BRIDGE_H
#define MATLAB_BRIDGE_H

#include <vector>
#include <iostream>
#include <fstream>

class mwArray;
class MatlabBridge {
public:
  MatlabBridge();
  ~MatlabBridge();

  static MatlabBridge* singleton();

  bool process();

  bool calibrate(const std::vector<float> &inReal,
                 const std::vector<float> &inImag,
                 const double inMJDTime,
                 const double inFreq,
                 const std::vector<int> &inUVFlags,
                 std::vector<float> &outReal,
                 std::vector<float> &outImag);

  bool createImage(const std::vector<float> &inReal,
                   const std::vector<float> &inImag,
                 const double inMJDTime,
                 const double inFreq,
                   const std::vector<float> &inULoc,
                   const std::vector<float> &inVLoc,
                   std::vector<float> &outSkymap,
                   std::vector<float> &outSkymapradec,
                   std::vector<float> &outVispad);

private:
  static MatlabBridge *sSingleton;
  void killMatlab();
  bool initMatlab();

  // Details of this timeslice.
  mwArray *mTime, *mFreq; 

  /*** Calibration related private members */
  // Gain solution pre-applied to all timeslices
  mwArray *mPreCalGain;
  // Solutions for this timeslice
  mwArray *mGain, *mSigmas, *mSigman; 
  // raw, calibrated and gridded visibilities
  mwArray *mVis, *mCalVis, *mVisPad;
  // Catalog + estimated source positions
  mwArray *mThSrcCat, *mPhiSrcCat, *mThSrcWsf, *mPhiSrcWsf;
  mwArray *mUVFlag, *mAntFlag, *mDebugLev, *mptSun, *mGoodCal, *mUVMask;
  mwArray *mSunComps; // CVX solar model extraction; pep/17Sep12

  /*** Imaging related private members */
  // Generate maps in local and RA/DEC coordinates
  mwArray *mSkyMap, *mSkyMapradec, *mSkyMapPrev, *mSkyMapDiff;
  mwArray *mUloc, *mVloc, *mDuv, *mNuv, *mUVSize;

  std::ofstream mMonFile, mSrcPosFile;
  int mBridgeId;
  int nAnts, nActiveAnts;
};

#endif // MATLAB_BRIDGE_H
