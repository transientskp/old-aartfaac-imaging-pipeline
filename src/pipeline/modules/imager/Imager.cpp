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
  Q_UNUSED(input);
  Q_UNUSED(output);
}
