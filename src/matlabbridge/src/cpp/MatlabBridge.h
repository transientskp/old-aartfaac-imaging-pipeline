#ifndef MATLAB_BRIDGE_H
#define MATLAB_BRIDGE_H

#include <vector>

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
                 const std::vector<float> &inUVFlags,
                 std::vector<float> &outReal,
                 std::vector<float> &outImag);

  bool createImage(const std::vector<float> &inReal,
                   const std::vector<float> &inImag,
                   const std::vector<float> &inULoc,
                   const std::vector<float> &inVLoc,
                   std::vector<float> &outSkymap,
                   std::vector<float> &outVispad);

private:
	static MatlabBridge *sSingleton;
  void killMatlab();
  bool initMatlab();
};

#endif // MATLAB_BRIDGE_H
