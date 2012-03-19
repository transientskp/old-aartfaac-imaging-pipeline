#ifndef IMAGER_MODULE_H
#define IMAGER_MODULE_H

#include <pelican/core/AbstractModule.h>

using namespace pelican;

class UniboardDataBlob;
class MatlabBridge;

class Imager : public AbstractModule {
public:
  Imager(const ConfigNode &inConfig);
  ~Imager();

  void run(const UniboardDataBlob *input, UniboardDataBlob *output);

private:
  std::vector<float> mULoc;
  std::vector<float> mVLoc;
  MatlabBridge *mBridge;

  void readData(const QString &inFilename, std::vector<float> &outData);
};

PELICAN_DECLARE_MODULE(Imager)

#endif // IMAGER_MODULE_H
