#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "modules/flagger/Flagger.h"
#include "../utilities/Utils.h"
#include "../utilities/monitoring/Server.h"
#include <time.h>

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
}

// Defines a single iteration of the pipeline.
void UniboardPipeline::run(QHash<QString, DataBlob *>& inRemoteData)
{
  mTimer.start();

  // Get pointers to the remote data blob(s) from the supplied hash.
  StreamBlob *data = static_cast<StreamBlob *>(inRemoteData["StreamBlob"]);

  // Start potential threads
  int channel = 0;
  const int num_channels = data->mNumChannels;

  #pragma omp parallel
  {
    #pragma omp single
    while (channel < num_channels)
    {
      #pragma omp task firstprivate(channel, data)
      {
        int idx = 0;
#ifdef ENABLE_OPENMP
        idx = omp_get_thread_num();
#endif // ENABLE_OPENMP

        // Flag bad antennas/visibilities
        mFlaggers[idx]->run(channel, data, data);
        // Calibrate correlations
        mCalibrators[idx]->run(channel, data, data);
      }

      channel++;
    }
  }

  // Create image
  mImager->run(data, data);

  float duration = (mTimer.elapsed() / 1000.0f);

  qDebug("Processed subband (%d-%d) in %0.3f sec",
         data->mHeader.start_chan, data->mHeader.end_chan, duration);

  ADD_STAT("CHUNKS", data->mHeader.time, (duration/num_channels));

  // Output to stream(s), see modules/output
  dataOutput(data, "post");
}
