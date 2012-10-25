#ifndef IMAGER_MODULE_H
#define IMAGER_MODULE_H

#include <pelican/core/AbstractModule.h>

using namespace pelican;

class StreamBlob;

class Imager : public AbstractModule
{
public:
  Imager(const ConfigNode &inConfig);
  ~Imager();

  void run(const StreamBlob *input, StreamBlob *output);

private:
  std::vector<float> mULoc;
  std::vector<float> mVLoc;

  void readData(const QString &inFilename, std::vector<float> &outData);
};

PELICAN_DECLARE_MODULE(Imager)

#endif // IMAGER_MODULE_H
