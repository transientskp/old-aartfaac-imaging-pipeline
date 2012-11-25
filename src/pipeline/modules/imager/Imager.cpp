#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"

#include <pelican/utility/Config.h>
#include <QtCore>

Imager::Imager(const ConfigNode &inConfig):
 AbstractModule(inConfig)
{
}

Imager::~Imager()
{
}

void Imager::run(const StreamBlob *input, StreamBlob *output)
{
  output->setMJDTime(input->getMJDTime());
  const std::vector<float> *real = input->getXXReal();
  const std::vector<float> *imag = input->getXXImag();
  std::vector<float> &skymap = output->getSkyMap();
  std::vector<float> &vismap = output->getVisMap();

  Q_UNUSED(real);
  Q_UNUSED(imag);
  Q_UNUSED(skymap);
  Q_UNUSED(vismap);
}
