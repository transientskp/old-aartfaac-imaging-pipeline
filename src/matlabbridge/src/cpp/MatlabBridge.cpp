#include "libmatlabbridge.h"
#include "MatlabBridge.h"

#include <mclmcrrt.h>
#include <mclcppclass.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <sstream>
using namespace std;

MatlabBridge* MatlabBridge::sSingleton = NULL;

MatlabBridge::MatlabBridge()
{ initMatlab();
  try 
  { mTime = new mwArray (1, 1, mxDOUBLE_CLASS, mxREAL);
    mFreq = new mwArray (1, 1, mxDOUBLE_CLASS, mxREAL);
    mGoodCal = new mwArray (1, 1, mxINT32_CLASS);
    mVis = new mwArray (288, 288, mxSINGLE_CLASS, mxCOMPLEX);
    mCalVis = new mwArray (288, 288, mxSINGLE_CLASS, mxCOMPLEX);
    mVisPad = new mwArray (1024, 1024, mxSINGLE_CLASS, mxCOMPLEX);
    mSkyMap = new mwArray (1024, 1024, mxSINGLE_CLASS, mxREAL);
    mSkyMapradec = new mwArray;
    mUVFlag = new mwArray (288, 288, mxINT32_CLASS);
    mDebugLev = new mwArray (1, 1, mxSINGLE_CLASS, mxREAL);
    mGain = new mwArray;
    mSigmas = new mwArray;
    mSigman = new mwArray;
    mThSrcCat = new mwArray;
    mPhiSrcCat = new mwArray;
    mThSrcWsf = new mwArray;
    mPhiSrcWsf = new mwArray;
    mPreCalGain = new mwArray (1, 288, mxSINGLE_CLASS, mxCOMPLEX);
  }
  catch (const mwException& e)
  { std::cerr << e.what() << std::endl; }
  catch (...)
  { std::cerr << "Unexpected error thrown" << std::endl; }     
  
  // Create respective Mon files
  time_t rawtime;
  struct tm *timeinfo;
  time (&rawtime);
  timeinfo = localtime ( &rawtime );
  stringstream gainfname, posfname; 
  gainfname <<"Mon_"<< timeinfo->tm_mday<<timeinfo->tm_mon<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_hour<<timeinfo->tm_min <<"_gains.txt";
  mMonFile.open (gainfname.str().c_str());
  posfname <<"Mon_"<< timeinfo->tm_mday<<timeinfo->tm_mon<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_hour<<timeinfo->tm_min <<"_srcpos.txt";
  mSrcPosFile.open (posfname.str().c_str());

  // Set Debug Level
  (*mDebugLev)(1) = 0;
  cerr << "MatlabBridge created: Debug level set to " << mDebugLev->Get(1,1) 
       << endl << endl;
}

MatlabBridge::~MatlabBridge()
{ if (mTime) delete mTime;
  if (mFreq) delete mFreq;
  if (mGoodCal) delete mGoodCal;
  if (mVis)  delete mVis;
  if (mCalVis) delete mCalVis;
  if (mVisPad) delete mVisPad;
  if (mSkyMap) delete mSkyMap;
  if (mSkyMapradec) delete mSkyMapradec;
  if (mGain) delete mGain;
  if (mSigmas) delete mSigmas;
  if (mSigman) delete mSigman;
  if (mUVFlag) delete mUVFlag;
  if (mDebugLev) delete mDebugLev;
  if (mThSrcCat) delete mThSrcCat;
  if (mPhiSrcCat) delete mPhiSrcCat;
  if (mThSrcWsf) delete mThSrcWsf;
  if (mPhiSrcWsf) delete mPhiSrcWsf;
  if (mPreCalGain) delete mPreCalGain;
  mMonFile.close();
  mSrcPosFile.close();

  killMatlab();
}

MatlabBridge* MatlabBridge::singleton()
{ if (sSingleton == NULL)
    sSingleton = new MatlabBridge();
  return sSingleton;
}


bool MatlabBridge::calibrate(const std::vector<float> &inReal,
                             const std::vector<float> &inImag,
                             const double inMJDTime,
                             const double inFreq,
                             const std::vector<int> &inUVFlags,
                             std::vector<float> &outReal,
                             std::vector<float> &outImag)
{ static int calrun = 0;
  int maxprecal = 5; // In number of runs

  // Setup the input
  mVis->Real().SetData(const_cast<float*>(&inReal[0]), inReal.size());
  mVis->Imag().SetData(const_cast<float*>(&inImag[0]), inImag.size());
  (*mTime)(1) = inMJDTime; (*mFreq)(1) = inFreq;
  /* cerr<<"#---> Time: "<< inMJDTime<< " inReal/inImag.size" << inReal.size() 
         << " "<<inImag.size();
     for (unsigned int i=0; i<inReal.size(); i++)
       cerr<<std::setprecision(10)<<inReal[i]<<" "<<inImag[i]<<endl;
     cerr<<endl<<endl;
  */

  // PerTimeslice flag
  mUVFlag->SetData(const_cast<int*>(&inUVFlags[0]), inUVFlags.size());

  // pelican_sunAteamsub (5, *mSkyMap, *mGain, *mSigmas, *mSigman, *mGoodCal, 
  //                      *mVis, *mTime, *mFreq, *mUVFlag, *mDebugLev);
  // pelican_pipesim_cpp (1, good, correlations, time, freq);

  // Within the precal part
  if (calrun < maxprecal)
  { pelican_sunAteamsub (9, *mThSrcCat, *mPhiSrcCat, *mThSrcWsf, *mPhiSrcWsf, 
                       *mCalVis, *mGain, *mSigmas, *mSigman, *mGoodCal, 
                       *mVis, *mTime, *mFreq, *mUVFlag, *mDebugLev);
    // Only store the good calibration runs
    if (mGoodCal->Get(1,1)) 
    { matadd (1, *mPreCalGain, *mPreCalGain, *mGain);
      if (++calrun == maxprecal)
      { cerr << "PreCalibration Done!\n";
	mwArray norm_fac (1, 1, mxSINGLE_CLASS);
        norm_fac (1,1) = maxprecal;
        // Normalize averaged gains
	normalize (1, *mPreCalGain, *mPreCalGain, norm_fac);
      }
    }
  }
  else
  { // Apply the precal to the data
    applycal (1, *mVis, *mVis, *mPreCalGain);
    pelican_sunAteamsub (9, *mThSrcCat, *mPhiSrcCat, *mThSrcWsf, *mPhiSrcWsf, 
                       *mCalVis, *mGain, *mSigmas, *mSigman, *mGoodCal, 
                       *mVis, *mTime, *mFreq, *mUVFlag, *mDebugLev);
  }

  mwArray re, im;
/*// NOTE: Not using outReal etc. to communicate across objects
  int width = mCalVis->GetDimensions().Get(1,1);
  int height = mCalVis->GetDimensions().Get(1,2);
  outReal.resize(width*height);
  outImag.resize(width*height);
  mCalVis->Real().GetData(&outReal[0], width*height);
  mCalVis->Imag().GetData(&outImag[0], width*height);
*/

  // Write out generated solutions
  int width = mGain->GetDimensions ().Get(1,1); 
  int height = mGain->GetDimensions ().Get(1,2);
  mMonFile << std::setw(10)<< std::setprecision(10)<< "# "<<inMJDTime<<"  "
           <<inFreq << endl;
  re = mGain->Real(); im = mGain->Imag();
  for (int i=1; i<=height; i++)
  { mMonFile << std::setw(10) << std::setprecision(10)<<"  "<< re.Get(1,i) 
    << "  " << im.Get(1,i) << endl; }
  mMonFile << endl << endl;
  mMonFile.flush ();

  // Write out WSF estimated positions
  width = mThSrcCat->GetDimensions().Get(1,1);
  height = mThSrcCat->GetDimensions().Get(1,2);
  mSrcPosFile<< std::setprecision(10)<<inMJDTime;
  for (int i=1; i<=width; i++)
  { mSrcPosFile <<" "<<mThSrcCat->Get(1,i)<<" "<<mPhiSrcCat->Get(1,i)
                <<" "<<mThSrcWsf->Get(1,i)<<" "<<mPhiSrcWsf->Get(1,i);
  }
  // Also write out estimated source fluxes of model sources.
  width = mSigmas->GetDimensions().Get(1,1);
  height = mSigmas->GetDimensions().Get(1,2);
  for (int i=1; i<=width; i++)
  { mSrcPosFile <<" "<<mSigmas->Get(1,i);
  }
  
  mSrcPosFile << endl;
  mSrcPosFile.flush ();
  return true;
}


bool MatlabBridge::createImage(const std::vector<float> &inReal,
                               const std::vector<float> &inImag,
                               const std::vector<float> &inULoc,
                               const std::vector<float> &inVLoc,
                               std::vector<float> &outSkymap,
                               std::vector<float> &outSkymapradec,
                               std::vector<float> &outVispad)
{
  (void) outVispad;

/*
  mwArray skymap, vispad, skymapradec;
  mwArray correlations(288*288, 1, mxSINGLE_CLASS, mxCOMPLEX);
  correlations.Real().SetData(const_cast<float*>(&inReal[0]), inReal.size());
  correlations.Imag().SetData(const_cast<float*>(&inImag[0]), inImag.size());
*/

  mwArray uloc(288*288, 1, mxSINGLE_CLASS);
  uloc.SetData(const_cast<float*>(&inULoc[0]), inULoc.size());

  mwArray vloc(288*288, 1, mxSINGLE_CLASS);
  vloc.SetData(const_cast<float*>(&inVLoc[0]), inVLoc.size());

  mwArray duv(1, 1, mxSINGLE_CLASS);
  mwArray nuv(1, 1, mxSINGLE_CLASS);
  mwArray uvsize(1, 1, mxSINGLE_CLASS);

  // Parameters to fft_imager_sjw_radec ()
  // duv(1) = 600.0f/256.0f;
  // nuv(1) = 512;
  // uvsize(1) = 512;

  // Parameters to fft_imager ()
  duv(1) = 1.5;
  nuv(1) = 500; // 1000;
  uvsize(1) = 512; // 1024;

  // fft_imager (2, *mSkyMap, *mVisPad, *mCalVis, uloc, vloc, duv, nuv, uvsize);
  fft_imager_sjw_radec (3, *mSkyMapradec, *mSkyMap, *mVisPad, *mCalVis, uloc, 
                        vloc, duv, nuv, uvsize, *mTime, *mFreq);

  // Write out an all-sky map in RA/DEC coordinates
  int width = mSkyMapradec->GetDimensions().Get(1,1);
  int height = mSkyMapradec->GetDimensions().Get(1,2);
  outSkymap.resize(width*height);
  mSkyMapradec->GetData(&outSkymapradec[0], width*height);

  // Write out an all-sky map in local coordinates
  width = mSkyMap->GetDimensions().Get(1,1);
  height = mSkyMap->GetDimensions().Get(1,2);
  outSkymap.resize(width*height);
  mSkyMap->GetData(&outSkymap[0], width*height);

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
