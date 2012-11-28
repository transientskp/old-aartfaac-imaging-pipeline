#ifndef UNIBOARD_PIPELINE_H
#define UNIBOARD_PIPELINE_H

#include <pelican/core/AbstractPipeline.h>

using namespace pelican;

class StreamBlob;
class Calibrator;
class Imager;
class Flagger;

class UniboardPipeline : public AbstractPipeline
{
public:
  /// Constructor.
  UniboardPipeline() : AbstractPipeline(),
    mFlagger(NULL),
    mCalibrator(NULL),
    mImager(NULL),
    mBlobCount(0)
  {}

  /// Initialises the pipeline.
  void init();

  /// Defines one iteration of the pipeline.
  void run(QHash<QString, DataBlob *>& remoteData);

private:
  Flagger *mFlagger;
  Calibrator *mCalibrator;
  Imager *mImager;

  quint64 mBlobCount;
};

#endif // UNIBOARD_PIPELINE_H
