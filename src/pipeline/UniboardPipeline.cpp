#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "ServiceBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "modules/flagger/Flagger.h"

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif // ENABLE_OPENMP

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
#ifdef ENABLE_OPENMP
  mThreads = omp_get_max_threads();
  qDebug("OpenMP threads: %d", mThreads);
#endif // ENABLE_OPENMP


  // Create the pipeline modules and any local data blobs.
  mFlaggers.resize(mThreads);
  mCalibrators.resize(mThreads);

  for (int i = 0; i < mThreads; i++)
  {
    mFlaggers[i] = static_cast<Flagger *>(createModule("Flagger"));
    mCalibrators[i] = static_cast<Calibrator *>(createModule("Calibrator"));
  }

  mImager = static_cast<Imager *>(createModule("Imager"));

  // Request remote data.
  requestRemoteData("StreamBlob");
//  requestRemoteData("ServiceBlob");
}

// Defines a single iteration of the pipeline.
void UniboardPipeline::run(QHash<QString, DataBlob *>& inRemoteData)
{
  mTimer.start();

  // Get pointers to the remote data blob(s) from the supplied hash.
  StreamBlob *data = static_cast<StreamBlob *>(inRemoteData["StreamBlob"]);

  // Start potential threads
  int channel = 0;
#pragma omp parallel
{
  #pragma omp single
  while (channel < data->mNumChannels)
  {
    #pragma omp task firstprivate(channel,data)
    {
      // Flag bad antennas/visibilities
      mFlaggers[channel%mThreads]->run(channel, data, data);
      // Calibrate correlations
      mCalibrators[channel%mThreads]->run(channel, data, data);
      // Output calibrated visibilities, see modules/output
      dataOutput(data, "calibrated");
    }

    channel++;
  }
}

  // Create image
  mImager->run(data, data);

  float time = (mTimer.elapsed() / 1000.0f);

  qDebug("Processed subband (%d-%d) in %0.3f sec",
         data->mHeader.start_chan, data->mHeader.end_chan, time);

  // Output to stream(s), see modules/output
  dataOutput(data, "post");
}
