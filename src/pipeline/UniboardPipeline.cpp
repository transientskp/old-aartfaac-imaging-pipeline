#include "UniboardPipeline.h"

#include "UniboardDataBlob.h"
#include "modules/imager/Imager.h"

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
  // Create the pipeline modules and any local data blobs.
  mOutputData = (UniboardDataBlob*) createBlob("UniboardDataBlob");
  mImager = (Imager*) createModule("Imager");

  // Request remote data.
  requestRemoteData("UniboardDataBlob");

  mBlobCount = 0;
}

// Defines a single iteration of the pipeline.
void UniboardPipeline::run(QHash<QString, DataBlob*>& inRemoteData)
{
  // Get pointers to the remote data blob(s) from the supplied hash.
  UniboardDataBlob* input_data = (UniboardDataBlob*) inRemoteData["UniboardDataBlob"];
  mImager->run(input_data, mOutputData);
  dataOutput(input_data, "Image");
  qDebug("Processed %4lldth blob with timestamp %s", ++mBlobCount, qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")));
}
