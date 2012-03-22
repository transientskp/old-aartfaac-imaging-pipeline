#include "libmatlabbridge.h"
#include "matlabbridge.h"

#include <mclmcrrt.h>
#include <mclcppclass.h>
#include <limits>

MatlabBridge::MatlabBridge()
{
  initMatlab();
}


MatlabBridge::~MatlabBridge()
{
  killMatlab();
}


bool MatlabBridge::callMatlab(const std::vector<float> &inReal,
                              const std::vector<float> &inImag,
                              const std::vector<float> &inULoc,
                              const std::vector<float> &inVLoc,
                              std::vector<float> &outSkymap,
                              std::vector<float> &outVispad)
{
  (void) outVispad;

  mwArray skymap, vispad;
  mwArray correlations(288*288, 1, mxSINGLE_CLASS, mxCOMPLEX);
  correlations.Real().SetData(const_cast<float*>(&inReal[0]), inReal.size());
  correlations.Imag().SetData(const_cast<float*>(&inImag[0]), inImag.size());

  mwArray uloc(288*288, 1, mxSINGLE_CLASS);
  uloc.SetData(const_cast<float*>(&inULoc[0]), inULoc.size());

  mwArray vloc(288*288, 1, mxSINGLE_CLASS);
  vloc.SetData(const_cast<float*>(&inVLoc[0]), inVLoc.size());

  mwArray duv(1, 1, mxSINGLE_CLASS);
  duv(1) = 600.0f/511.0f;

  mwArray nuv(1, 1, mxSINGLE_CLASS);
  nuv(1) = 512;

  mwArray uvsize(1, 1, mxSINGLE_CLASS);
  uvsize(1) = 512;

  fft_imager(2, skymap, vispad, correlations, uloc, vloc, duv, nuv, uvsize);

  mwArray dimensions = skymap.GetDimensions();
  int width = dimensions(1,1);
  int height = dimensions(1,2);
  outSkymap.resize(width*height);
  skymap.GetData(&outSkymap[0], width*height);

  return true;
}


bool MatlabBridge::initMatlab()
{
  // initialise matlab compiler runtime
  mclmcrInitialize();

  const char *args[] = { "-nodisplay" };
  if (mclInitializeApplication(args, 1)) {
    std::cout << "matlab runtime initialized" << std::endl;
  } else {
    std::cerr << "An error occurred while initializing matlab application";
    std::cerr << mclGetLastErrorMessage() << std::endl;
    return false;
  }

  if (libmatlabbridgeInitialize()) {
    std::cout << "matlab bridge initialized" << std::endl;
    return true;
  } else {
    std::cout << "can't init matlab bridge!" << std::endl;
    libmatlabbridgePrintStackTrace();
    return false;
  }
}


void MatlabBridge::killMatlab()
{
  libmatlabbridgeTerminate();
  mclTerminateApplication();
}
