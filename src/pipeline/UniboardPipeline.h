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
  UniboardPipeline(int threads=1) : AbstractPipeline(),
    mImager(NULL),
    mThreads(threads)
  {}

  /// Initialises the pipeline.
  void init();

  /// Defines one iteration of the pipeline.
  void run(QHash<QString, DataBlob *>& remoteData);

private:
  std::vector<Flagger*> mFlaggers;
  std::vector<Calibrator*> mCalibrators;

  Imager *mImager;
  int mThreads;
};

#endif // UNIBOARD_PIPELINE_H
