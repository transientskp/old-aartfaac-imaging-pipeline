#ifndef UNIBOARD_PIPELINE_H
#define UNIBOARD_PIPELINE_H

#include <pelican/core/AbstractPipeline.h>

using namespace pelican;

class StreamBlob;
class Calibrator;
class Imager;

class UniboardPipeline : public AbstractPipeline
{
public:
  /// Constructor.
  UniboardPipeline() : AbstractPipeline(),
      mOutputData(NULL),
      mImager(NULL),
      mCalibrator(NULL),
      mBlobCount(0)
  {}

  /// Initialises the pipeline.
  void init();

  /// Defines one iteration of the pipeline.
  void run(QHash<QString, DataBlob*>& remoteData);

private:
  StreamBlob *mOutputData;
  Imager *mImager;
  Calibrator *mCalibrator;
  quint64 mBlobCount;
};

#endif // UNIBOARD_PIPELINE_H
