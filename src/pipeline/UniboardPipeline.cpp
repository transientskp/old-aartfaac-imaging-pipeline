#include "UniboardPipeline.h"

#include "UniboardDataBlob.h"
#include "modules/imager/Imager.h"
#include "modules/calibrator/Calibrator.h"
#include "../emulator/UdpPacket.h"

// Initialises the pipeline, creating required modules and data blobs,
// and requesting remote data.
void UniboardPipeline::init()
{
  // Create the pipeline modules and any local data blobs.
  mOutputData = (UniboardDataBlob*) createBlob("UniboardDataBlob");
  mImager = (Imager*) createModule("Imager");
  mCalibrator = (Calibrator*) createModule("Calibrator");

  // Request remote data.
  requestRemoteData("UniboardDataBlob");

  mBlobCount = 0;
}

// Defines a single iteration of the pipeline.
void UniboardPipeline::run(QHash<QString, DataBlob*>& inRemoteData)
{ static int miss = 0;
  int stride = 1, skiprecs = 0;
  // Get pointers to the remote data blob(s) from the supplied hash.
  UniboardDataBlob* input_data = 
                         (UniboardDataBlob*) inRemoteData["UniboardDataBlob"];

  if (miss++ < skiprecs) 
  { qDebug("Skipped %4lldth blob with timestamp %s", ++mBlobCount, 
        qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")));
    return;
  }

  if (miss % stride != 0)
  { qDebug("Skipped %4lldth blob with timestamp %s", ++mBlobCount, 
        qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")));
  }
  else
  {
    // Calibrate correlations
    mCalibrator->run(input_data, mOutputData);

    // Create image
    mImager->run(mOutputData, mOutputData);

    // Output to stream(s)
    dataOutput(mOutputData, "post");
    qDebug("Processed %4lldth blob with timestamp %s, written to file %s.", 
           ++mBlobCount, 
        qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")), 
        qPrintable(input_data->getDateTime().toString("dd-MM-yyyy hh:mm:ss")));
  }
}
