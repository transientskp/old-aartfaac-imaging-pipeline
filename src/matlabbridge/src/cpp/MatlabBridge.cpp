#include "libmatlabbridge.h"
#include "MatlabBridge.h"

#include <mclmcrrt.h>
#include <mclcppclass.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

MatlabBridge* MatlabBridge::sSingleton = NULL;

MatlabBridge::MatlabBridge()
{ initMatlab();
  try 
  { mAntFlag = new mwArray (1,2,mxINT32_CLASS); 
    (*mAntFlag)(1) = 6; (*mAntFlag)(2) =103;
    nAnts = 288; nActiveAnts = nAnts - 2;
    // Use when no flagging needed.
    // mAntFlag = new mwArray;
    // nAnts = 288; nActiveAnts = nAnts;

    mTime = new mwArray (1, 1, mxDOUBLE_CLASS, mxREAL);
    mFreq = new mwArray (1, 1, mxDOUBLE_CLASS, mxREAL);
    mGoodCal = new mwArray (1, 1, mxINT32_CLASS);
    mVis = new mwArray (nAnts, nAnts, mxSINGLE_CLASS, mxCOMPLEX);
    mCalVis = new mwArray (nActiveAnts, nActiveAnts, mxSINGLE_CLASS, mxCOMPLEX);
    mVisPad = new mwArray (1024, 1024, mxSINGLE_CLASS, mxCOMPLEX);
    mSkyMap = new mwArray (1024, 1024, mxSINGLE_CLASS, mxREAL);
    mSkyMapPrev = new mwArray (1024, 1024, mxSINGLE_CLASS, mxREAL);
    mSkyMapDiff = new mwArray; //  (1024, 1024, mxSINGLE_CLASS, mxREAL);
    mSkyMapradec = new mwArray;
    mUVFlag = new mwArray (nAnts, nAnts, mxINT32_CLASS);
    mUVMask = new mwArray (nAnts, nAnts, mxLOGICAL_CLASS);
    mDebugLev = new mwArray (1, 1, mxSINGLE_CLASS, mxREAL);
    mptSun = new mwArray (1, 1, mxINT32_CLASS);
    mGain = new mwArray;
    mSigmas = new mwArray;
    mSigman = new mwArray;
    mThSrcCat = new mwArray;
    mPhiSrcCat = new mwArray;
    mThSrcWsf = new mwArray;
    mPhiSrcWsf = new mwArray;
    mPreCalGain = new mwArray (1, nActiveAnts, mxSINGLE_CLASS, mxCOMPLEX);
    mUloc = new mwArray (1, nActiveAnts*nActiveAnts, mxSINGLE_CLASS);
    mVloc = new mwArray (1, nActiveAnts*nActiveAnts, mxSINGLE_CLASS);
    mDuv = new mwArray (1, 1, mxSINGLE_CLASS);
    (*mDuv)(1) = 1.5;
    mNuv = new mwArray (1, 1, mxSINGLE_CLASS);
    (*mNuv)(1) = 500;
    mUVSize = new mwArray (1, 1, mxSINGLE_CLASS);
    (*mUVSize)(1) = 512;
    mSunComps = new mwArray; // CVX Solar model extraction, pep/17Sep12
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
  mBridgeId = getpid();

  stringstream gainfname, posfname; 
  gainfname <<"Mon_"<< timeinfo->tm_mday<<timeinfo->tm_mon<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_hour<<timeinfo->tm_min <<"_gains_" <<mBridgeId<< ".txt";
/*
  struct stat st;
  int id = 0;
  if (!stat (gainfname.str().c_str(), &st)) // file exists! try opening another
  { id++;
    do
    { gainfname.str("");
      gainfname <<"Mon_"<< timeinfo->tm_mday<<timeinfo->tm_mon<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_hour<<timeinfo->tm_min <<"_gains" <<id<< ".txt";
    }
    while (!stat (gainfname.str().c_str(), &st));
  }
*/
  mMonFile.open (gainfname.str().c_str());
  posfname <<"Mon_"<< timeinfo->tm_mday<<timeinfo->tm_mon<<timeinfo->tm_year+1900<<"_"<<timeinfo->tm_hour<<timeinfo->tm_min <<"_srcpos_"<<mBridgeId<<".txt"; // Use previous id
  mSrcPosFile.open (posfname.str().c_str());
  // NOTE: Columns are defined within the calibration/imaging routines, so make 
  // sure file header matches what is actually printed!
  // mSrcPosFile << "# Cols: Time, norm_acc, norm_uvmaskacc, thsrccat, phisrccat, thsrcwsf, phisrcwsf, multiple sigmas, norm_calvis, norm_sigman, norm_gain, norm_calmap" << endl;
  // NOTE: By default, the position estimates and flux estimates of all the
  // model sources (CasA, CygA, TauA, VirA and sun) are printed, with 0 values 
  // if any source is not above the horizon, or if it is very weak.
  mSrcPosFile << "\"Time (MJD secs)\" \"Freq (Hz)\" \"Raw vis. norm\" \"UVmask vis. norm\" \"CasA cat. th\" \"CasA cat. phi\" \"CasA wsf th\" \"CasA wsf phi\" \"CygA cat. th\" \"CygA cat. phi\" \"CygA wsf th\" \"CygA wsf phi\" \"TauA cat. th\" \"TauA cat. phi\" \"TauA wsf th\" \"TauA wsf phi\" \"VirA cat. th\" \"VirA cat. phi\" \"VirA wsf th\" \"VirA wsf phi\" \"Sun cat. th\" \"Sun cat. phi\" \"Sun wsf th\" \"Sun wsf phi\" \"CasA flux\" \"CygA flux\" \"TauA flux\" \"VirA flux\" \"Sun flux\" \"Calvis norm\" \"Noise norm\" \"gain norm\" \"calmap norm\"" << endl;

  // Set Debug Level
  (*mDebugLev)(1) = 3;
  cerr << "MatlabBridge created: Debug level set to " << mDebugLev->Get(1,1) 
       << endl << endl;
  // Choose point source or sparse reconstruction modelling of the Sun.
  (*mptSun) (1) = 1;

  // Create Telescope information object
}

MatlabBridge::~MatlabBridge()
{ if (mTime) delete mTime;
  if (mAntFlag) delete mAntFlag;
  if (mFreq) delete mFreq;
  if (mGoodCal) delete mGoodCal;
  if (mVis)  delete mVis;
  if (mCalVis) delete mCalVis;
  if (mVisPad) delete mVisPad;
  if (mSkyMap) delete mSkyMap;
  if (mSkyMapPrev) delete mSkyMapPrev;
  if (mSkyMapDiff) delete mSkyMapDiff;
  if (mSkyMapradec) delete mSkyMapradec;
  if (mGain) delete mGain;
  if (mSigmas) delete mSigmas;
  if (mSigman) delete mSigman;
  if (mUVFlag) delete mUVFlag;
  if (mUVMask) delete mUVMask;
  if (mDebugLev) delete mDebugLev;
  if (mptSun) delete mptSun;
  if (mThSrcCat) delete mThSrcCat;
  if (mPhiSrcCat) delete mPhiSrcCat;
  if (mThSrcWsf) delete mThSrcWsf;
  if (mPhiSrcWsf) delete mPhiSrcWsf;
  if (mPreCalGain) delete mPreCalGain;
  if (mUloc) delete mUloc;
  if (mVloc) delete mVloc;
  if (mDuv) delete mDuv; 
  if (mNuv) delete mNuv; 
  if (mUVSize) delete mUVSize;
  if (mSunComps) delete mSunComps;
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
  // mUVFlag->SetData(const_cast<int*>(&inUVFlags[0]), inUVFlags.size());
  // NOTE: Currently hand setting the flags to ignore autocorrelations, antenna
  // number ?? and ??. Using till the time a flagger module passes on currect 
  // flagging.
/*
  for (int i=1; i<=nAnts; i++)
  { (*mUVFlag) (i,i)  = 1; // 1 => ignore visibility, 0 => use visibility
    (*mUVFlag) (103,i) = 1; 
    (*mUVFlag) (i,103) = 1; // NOTE: Matlab like 1-ref. ind
    (*mUVFlag) (6,i) = 1; 
    (*mUVFlag) (i,6) = 1;
  }
*/
  // pelican_sunAteamsub (5, *mSkyMap, *mGain, *mSigmas, *mSigman, *mGoodCal, 
  //                      *mVis, *mTime, *mFreq, *mUVFlag, *mDebugLev);
  // pelican_pipesim_cpp (1, good, correlations, time, freq);

  mwArray norm, maskvis, flag_ant (1, 2, mxINT32_CLASS);
  matelemul (1, maskvis, *mUVMask, *mVis);
  mSrcPosFile<< std::setprecision(10)<<inMJDTime << " " << inFreq;
  frobenius_norm (1, norm, *mVis);   mSrcPosFile << " "<<norm.Get (1,1);
  frobenius_norm (1, norm, maskvis); mSrcPosFile << " "<<norm.Get (1,1);

  // Within the precal part
  if (calrun < maxprecal)
  { if (calrun == 0)
    { mwArray restriction (1, 1, mxINT32_CLASS);
      restriction (1) = 10;
      cal_uvmask (1, *mUVMask, restriction, *mFreq);
      flag_ant(1,1) = 3; flag_ant (1,2) = 106;
    }
    pelican_sunAteamsub (10, *mThSrcCat, *mPhiSrcCat, *mThSrcWsf, *mPhiSrcWsf, 
                   *mSunComps, *mCalVis, *mGain, *mSigmas, *mSigman, *mGoodCal, 
                       *mVis, *mTime, *mFreq, *mUVFlag, *mAntFlag, *mDebugLev, *mptSun);
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
    // applycal (1, *mVis, *mVis, *mPreCalGain);
    pelican_sunAteamsub (10, *mThSrcCat, *mPhiSrcCat, *mThSrcWsf, *mPhiSrcWsf, 
                   *mSunComps, *mCalVis, *mGain, *mSigmas, *mSigman, *mGoodCal, 
                       *mVis, *mTime, *mFreq, *mUVFlag, *mAntFlag, *mDebugLev, *mptSun);
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
  for (int i=1; i<=width; i++)
  { mSrcPosFile <<" "<<std::setprecision(10)<<mThSrcCat->Get(1,i)<<" "<<mPhiSrcCat->Get(1,i) <<" "<<mThSrcWsf->Get(1,i)<<" "<<mPhiSrcWsf->Get(1,i);
  }
  // Also write out estimated source fluxes of model sources.
  width = mSigmas->GetDimensions().Get(1,1);
  height = mSigmas->GetDimensions().Get(1,2);
  for (int i=1; i<=height; i++)
  { mSrcPosFile <<" "<<mSigmas->Get(1,i);
  }
  // ... and norms of various inputs
  frobenius_norm (1, norm, *mCalVis); mSrcPosFile << " "<<norm.Get (1,1);
  frobenius_norm (1, norm, *mSigman); mSrcPosFile << " "<<norm.Get (1,1);
  frobenius_norm (1, norm, *mGain);   mSrcPosFile << " "<<norm.Get (1,1);
  // Writing some more in the imager routine...
  // mSrcPosFile << endl;
  // mSrcPosFile.flush ();
  return true;
}


bool MatlabBridge::createImage(const std::vector<float> &inReal,
                               const std::vector<float> &inImag,
                 	       const double inMJDTime,
                               const double inFreq,
                               const std::vector<float> &inULoc,
                               const std::vector<float> &inVLoc,
                               std::vector<float> &outSkymap,
                               std::vector<float> &outSkymapradec,
                               std::vector<float> &outVispad)
{ static int first = 1;
  (void) outVispad;
  

/*
  mwArray skymap, vispad, skymapradec;
  mwArray correlations(288*288, 1, mxSINGLE_CLASS, mxCOMPLEX);
  correlations.Real().SetData(const_cast<float*>(&inReal[0]), inReal.size());
  correlations.Imag().SetData(const_cast<float*>(&inImag[0]), inImag.size());
*/

  if (first)
  { // call matlab function to separate out unflagged uloc, vloc, which 
    // fills in the private uloc, vloc.
    first = 0;
    cout << "---> FIRST IMAGER CALL! SETTING UP ULOC/VLOC..."<<endl;
    cout << "Antflag dimensions: " << mAntFlag->GetDimensions().Get(1,1)
         << " and " << mAntFlag->GetDimensions().Get(1,2) << endl;
    // if (!mAntFlag->IsEmpty()) // No flagging!
    if (!mAntFlag->GetDimensions().Get(1,2)) // No flagging, 0 sized array!
    { mUloc->SetData(const_cast<float*>(&inULoc[0]), inULoc.size());
      mVloc->SetData(const_cast<float*>(&inVLoc[0]), inVLoc.size());
    }
    else
    { cout << "-->Applying flagging on UV ITRF coordinates..\n";
      mwArray uloc(1, nAnts*nAnts, mxSINGLE_CLASS);
      uloc.SetData(const_cast<float*>(&inULoc[0]), inULoc.size());
      mwArray vloc(1, nAnts*nAnts, mxSINGLE_CLASS);
      vloc.SetData(const_cast<float*>(&inVLoc[0]), inVLoc.size());
      gen_flagged_uvloc (2, *mUloc, *mVloc, uloc, vloc, *mAntFlag);
    }
  }

/*
      mwArray uloc(1, nAnts*nAnts, mxSINGLE_CLASS);
      uloc.SetData(const_cast<float*>(&inULoc[0]), inULoc.size());
      mwArray vloc(1, nAnts*nAnts, mxSINGLE_CLASS);
      vloc.SetData(const_cast<float*>(&inVLoc[0]), inVLoc.size());
*/
  mwArray duv(1, 1, mxSINGLE_CLASS);
  mwArray nuv(1, 1, mxSINGLE_CLASS);
  mwArray uvsize(1, 1, mxSINGLE_CLASS);
  mwArray radec (1, 1, mxINT32_CLASS);
  (*mTime)(1) = inMJDTime; (*mFreq)(1) = inFreq;
  std::cerr << "fft_imaging: Time/Freq: " <<inFreq<<" "<<inMJDTime << std::endl;

  // Parameters to fft_imager_sjw_radec ()
  // duv(1) = 600.0f/256.0f;
  // nuv(1) = 512;
  // uvsize(1) = 512;

  // Parameters to fft_imager ()
  // duv(1) = 1.2;
  duv(1) = 2.5;
  nuv(1) = 500; // 1000;
  uvsize(1) = 512; // 1024;
  radec(1) = 0;

  // fft_imager (2, *mSkyMap, *mVisPad, *mCalVis, uloc, vloc, duv, nuv, uvsize);
  // fft_imager_sjw_radec (3, *mSkyMapradec, *mSkyMap, *mVisPad, *mCalVis, *mUloc,
   //                       *mVloc, duv, nuv, uvsize, *mTime, *mFreq);
  fft_imager_sjw_radec (3, *mSkyMapradec, *mSkyMap, *mVisPad, *mCalVis, *mUloc,
                         *mVloc, duv, nuv, uvsize, *mTime, *mFreq, radec);

  // Write out image statistics to file, continuing from calibrate function
  mwArray norm;
  frobenius_norm (1, norm, *mSkyMap);    mSrcPosFile << " "<<norm.Get (1,1);
  mSrcPosFile << endl;
  mSrcPosFile.flush ();

  // Generate temporal image difference
  // mwArray mapdiff;
  // imgdiff (1, *mSkyMapDiff, *mSkyMap, *mSkyMapPrev, *mTime, *mFreq);
  // imgdiff (1, mapdiff, *mSkyMap, *mSkyMapPrev, *mTime, *mFreq);

  
  // Write out an all-sky map in RA/DEC coordinates
  int width = mSkyMapradec->GetDimensions().Get(1,1);
  int height = mSkyMapradec->GetDimensions().Get(1,2);
  /* outSkymapradec.resize(width*height);
  mSkyMapradec->GetData(&outSkymapradec[0], width*height);
  */

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
