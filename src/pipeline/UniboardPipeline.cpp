#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "ServiceBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "modules/flagger/Flagger.h"

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
  // Create the pipeline modules and any local data blobs.
  mImager = static_cast<Imager *>(createModule("Imager"));
  mCalibrator = static_cast<Calibrator *>(createModule("Calibrator"));
  mFlagger = static_cast<Flagger *>(createModule("Flagger"));

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

  // Flag bad antennas/visibilities
  mFlagger->run(data, data);

  // Calibrate correlations
  mCalibrator->run(data, data);

  // Output calibrated visibilities, see modules/output
  dataOutput(data, "calibrated");

  // Create image
  mImager->run(data, data);

  float time = (mTimer.elapsed() / 1000.0f);

  qDebug("Processed %0.3f Hz in %0.3f sec", qPrintable(data->mFrequency, time);

  // Output to stream(s), see modules/output
  dataOutput(data, "post");
}
