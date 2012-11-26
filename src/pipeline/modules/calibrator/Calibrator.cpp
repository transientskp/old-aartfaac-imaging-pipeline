#include "Calibrator.h"

#include "../../StreamBlob.h"
#include "../../../Constants.h"

#include <pelican/utility/Config.h>
#include <QtCore>

Calibrator::Calibrator(const ConfigNode &inConfig):
  AbstractModule(inConfig)
{
}

Calibrator::~Calibrator()
{
}

void Calibrator::run(const StreamBlob *input, StreamBlob *output)
{
  Q_UNUSED(input);
  Q_UNUSED(output);
}
