#ifndef MATLAB_BRIDGE_H
#define MATLAB_BRIDGE_H

#include <vector>
#include <complex>

class mwArray;

class MatlabBridge {
public:
  MatlabBridge();
  ~MatlabBridge();
  bool process();

  bool callMatlab(const std::vector<float> &inReal,
                  const std::vector<float> &inImag,
                  const std::vector<float> &inULoc,
                  const std::vector<float> &inVLoc,
                  std::vector<float> &outSkymap,
                  std::vector<float> &outVispad);

private:
  void killMatlab();
  bool initMatlab();
};

#endif // MATLAB_BRIDGE_H
