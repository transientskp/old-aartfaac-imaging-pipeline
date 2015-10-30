#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "modules/flagger/Flagger.h"
#include "../utilities/Utils.h"
#include "Constants.h"
#include <time.h>
#include <sstream>

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif // ENABLE_OPENMP

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
#ifdef ENABLE_OPENMP
  mThreads = NUM_USED_POLARIZATIONS;
  omp_set_num_threads(mThreads);
  qDebug("OpenMP Enabled - %i threads", mThreads);
#else
  mThreads = 1;
  qDebug("OpenMP Disabled - %i thread", mThreads);
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

  #pragma omp parallel for
  for (quint32 p = 0; p < NUM_USED_POLARIZATIONS; p++)
  {
    int tid = 0;

    #ifdef ENABLE_OPENMP
    tid = omp_get_thread_num();
    #endif

    mFlaggers[tid]->run(p, data, data);
    mCalibrators[tid]->run(p, data, data);
  }

  // Create image
  mImager->run(data, data);

  // Don't add dataOutput to duration as we currently write to disk
  float duration = (mTimer.elapsed() / 1000.0f);

  // Output to stream(s), see modules/output
  dataOutput(data, "post");

  float bps = data->mNumChannels*NUM_BASELINES*NUM_USED_POLARIZATIONS*64 / duration;
  qDebug("[%s] processed (%d-%d) channels in %0.3f sec - %0.2f Mb/s",
         qPrintable(utils::MJD2QDateTime(data->mHeader.time).toString("hh:mm:ss")),
         data->mHeader.start_chan, data->mHeader.end_chan, duration, bps*1e-6f);
}
