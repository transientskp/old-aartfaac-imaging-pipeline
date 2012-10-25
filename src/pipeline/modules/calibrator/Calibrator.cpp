#include "Calibrator.h"

#include "../../StreamBlob.h"

#include <pelican/utility/Config.h>
#include <QtCore>

Calibrator::Calibrator(const ConfigNode &inConfig)
  : AbstractModule(inConfig)
{
  mUVFlags.resize(288 * 288, 0);
}

Calibrator::~Calibrator()
{
}

void Calibrator::run(const StreamBlob *input, StreamBlob *output)
{
  output->setMJDTime(input->getMJDTime());
  output->setFrequency(input->getFrequency());

  const std::vector<float> *inp_real = input->getXXReal();
  const std::vector<float> *inp_imag = input->getXXImag();

  std::vector<float> *out_real = output->getXXReal();
  std::vector<float> *out_imag = output->getXXImag();
  Q_UNUSED(inp_real);
  Q_UNUSED(inp_imag);
  Q_UNUSED(out_real);
  Q_UNUSED(out_imag);
}
