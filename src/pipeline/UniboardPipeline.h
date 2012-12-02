#ifndef UNIBOARD_PIPELINE_H
#define UNIBOARD_PIPELINE_H

#include <pelican/core/AbstractPipeline.h>
#include <QtCore>

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
    mImager(NULL)
  {}

  /// Initialises the pipeline.
  void init();

  /// Defines one iteration of the pipeline.
  void run(QHash<QString, DataBlob *>& remoteData);

private:
  Flagger *mFlagger;
  Calibrator *mCalibrator;
  Imager *mImager;

  QTime mTimer;
};

#endif // UNIBOARD_PIPELINE_H
