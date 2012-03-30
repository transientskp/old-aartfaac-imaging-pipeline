#ifndef CALIBRATOR_MODULE_H
#define CALIBRATOR_MODULE_H

#include <pelican/core/AbstractModule.h>

using namespace pelican;

class UniboardDataBlob;
class MatlabBridge;

class Calibrator : public AbstractModule {
public:
  Calibrator(const ConfigNode &inConfig);
  ~Calibrator();

  void run(const UniboardDataBlob *input, UniboardDataBlob *output);

private:
  std::vector<int> mUVFlags;
  MatlabBridge *mBridge;
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
