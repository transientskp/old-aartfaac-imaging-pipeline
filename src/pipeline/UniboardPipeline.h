#ifndef SIGNALPROCESSINGPIPELINE_H
#define SIGNALPROCESSINGPIPELINE_H

#include <pelican/core/AbstractPipeline.h>

using namespace pelican;

class UniboardDataBlob;
class Calibrator;
class Imager;

class UniboardPipeline : public AbstractPipeline
{
public:
  /// Constructor.
  UniboardPipeline() : AbstractPipeline() {}

  /// Initialises the pipeline.
  void init();

  /// Defines one iteration of the pipeline.
  void run(QHash<QString, DataBlob*>& remoteData);

private:
  UniboardDataBlob *mOutputData;
  Imager *mImager;
  Calibrator *mCalibrator;
  quint64 mBlobCount;
};

#endif // SIGNALPROCESSINGPIPELINE_H
