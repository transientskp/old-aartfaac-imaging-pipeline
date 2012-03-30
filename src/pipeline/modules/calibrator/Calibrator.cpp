#include "Calibrator.h"

#include "../../UniboardDataBlob.h"
#include "../../../matlabbridge/src/cpp/MatlabBridge.h"

#include <pelican/utility/Config.h>
#include <QtCore>

Calibrator::Calibrator(const ConfigNode &inConfig)
  : AbstractModule(inConfig)
{
  mUVFlags.resize(288*288, 0);
  mBridge = MatlabBridge::singleton();
}

Calibrator::~Calibrator()
{
  //delete mBridge;
}

void Calibrator::run(const UniboardDataBlob *input, UniboardDataBlob *output)
{
  output->setMJDTime(input->getMJDTime());
  output->setFrequency(input->getFrequency());

  const std::vector<float> *inp_real = input->getXXReal();
  const std::vector<float> *inp_imag = input->getXXImag();

  std::vector<float> *out_real = output->getXXReal();
  std::vector<float> *out_imag = output->getXXImag();
  mBridge->calibrate(*inp_real,
                     *inp_imag,
                     input->getMJDTime(),
                     input->getFrequency(),
                     mUVFlags,
                     *out_real,
                     *out_imag);
}
