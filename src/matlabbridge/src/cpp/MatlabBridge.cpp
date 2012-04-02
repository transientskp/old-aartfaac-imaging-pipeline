#include "libmatlabbridge.h"
#include "MatlabBridge.h"

#include <mclmcrrt.h>
#include <mclcppclass.h>

MatlabBridge* MatlabBridge::sSingleton = NULL;

MatlabBridge::MatlabBridge()
{
  initMatlab();
}


MatlabBridge::~MatlabBridge()
{
  killMatlab();
}

MatlabBridge* MatlabBridge::singleton()
{
  if (sSingleton == NULL)
    sSingleton = new MatlabBridge();
  return sSingleton;
}

//[calvis, gainsol, sigmas, sigman, good] = pelican_calib (acc, t_obs, freq, uvflag)
bool MatlabBridge::calibrate(const std::vector<float> &inReal,
                             const std::vector<float> &inImag,
                             const double inMJDTime,
                             const double inFreq,
                             const std::vector<int> &inUVFlags,
                             std::vector<float> &outReal,
                             std::vector<float> &outImag)
{
  mwArray skymap;
  mwArray correlations(288, 288, mxSINGLE_CLASS, mxCOMPLEX);
  correlations.Real().SetData(const_cast<float*>(&inReal[0]), inReal.size());
  correlations.Imag().SetData(const_cast<float*>(&inImag[0]), inImag.size());

  mwArray time(1, 1, mxDOUBLE_CLASS);
  time(1) = inMJDTime;

  mwArray freq(1, 1, mxDOUBLE_CLASS);
  freq(1) = inFreq;

  mwArray uv_flags(288, 288, mxINT32_CLASS);
  uv_flags.SetData(const_cast<int*>(&inUVFlags[0]), inUVFlags.size());

  mwArray gain, sigmas, sigman, good;
  pelican_calib(5, skymap, gain, sigmas, sigman, good, correlations, time, freq, uv_flags);

  mwArray dimensions = skymap.GetDimensions();
  int width = dimensions(1,1);
  int height = dimensions(1,2);
  outReal.resize(width*height);
  outImag.resize(width*height);
  skymap.Real().GetData(&outReal[0], width*height);
  skymap.Imag().GetData(&outImag[0], width*height);
  return true;
}


bool MatlabBridge::createImage(const std::vector<float> &inReal,
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
  duv(1) = 600.0f/256.0f;

  mwArray nuv(1, 1, mxSINGLE_CLASS);
  nuv(1) = 512;

  mwArray uvsize(1, 1, mxSINGLE_CLASS);
  uvsize(1) = 512;

  fft_imager_sjw(2, skymap, vispad, correlations, uloc, vloc, duv, nuv, uvsize);

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
