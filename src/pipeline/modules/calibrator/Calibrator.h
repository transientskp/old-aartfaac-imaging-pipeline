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
  std::vector<float> mULoc;
  std::vector<float> mVLoc;
  MatlabBridge *mBridge;

  void readData(const QString &inFilename, std::vector<float> &outData);
};

PELICAN_DECLARE_MODULE(Calibrator)

#endif // CALIBRATOR_MODULE_H
