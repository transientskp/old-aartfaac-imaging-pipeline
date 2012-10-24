#include "UniboardPipeline.h"

#include "StreamBlob.h"
#include "ServiceBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
  // Create the pipeline modules and any local data blobs.
  mOutputData = static_cast<StreamBlob*>(createBlob("StreamBlob"));
  mImager = static_cast<Imager*>(createModule("Imager"));
  mCalibrator = static_cast<Calibrator*>(createModule("Calibrator"));

  // Request remote data.
  requestRemoteData("StreamBlob");
  requestRemoteData("ServiceBlob");

  mBlobCount = 0;
}

// Defines a single iteration of the pipeline.
void UniboardPipeline::run(QHash<QString, DataBlob*>& inRemoteData)
{
  // Get pointers to the remote data blob(s) from the supplied hash.
  StreamBlob* input_data = static_cast<StreamBlob*>(inRemoteData["StreamBlob"]);

  // Calibrate correlations
  mCalibrator->run(input_data, mOutputData);

  // Create image
  mImager->run(mOutputData, mOutputData);

  // Output to stream(s), see modules/output
  dataOutput(mOutputData, "post");
  qDebug("Processed %4lldth blob with timestamp %s", ++mBlobCount, qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")));
}
