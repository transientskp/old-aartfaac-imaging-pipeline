#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "modules/flagger/Flagger.h"
#include "../utilities/Utils.h"
#include "../utilities/monitoring/Server.h"
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
  omp_set_dynamic(0);
  omp_set_num_threads(mThreads);
  qDebug("OpenMP threads: %d", mThreads);
#else
  mThreads = 1;
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

  mFlaggers[0]->run(XX_POL, data, data);
  mCalibrators[0]->run(XX_POL, data, data);

  // Create image
  mImager->run(data, data);

  float duration = (mTimer.elapsed() / 1000.0f);

  qDebug("Processed `%s' subband (%d-%d) in %0.3f sec",
         qPrintable(utils::MJD2QDateTime(data->mHeader.time).toString("hh:mm:ss")),
         data->mHeader.start_chan, data->mHeader.end_chan, duration);

  ADD_STAT("PERFORMANCE", data->mHeader.time, duration);

  // Output to stream(s), see modules/output
  dataOutput(data, "post");
}
