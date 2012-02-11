#include "UniboardEmulator.h"

#include <pelican/utility/ConfigNode.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Slicer.h>
#include <casacore/casa/Arrays/ArrayMath.h>

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &configNode)
  : AbstractUdpEmulator(configNode)
{
  _counter = 0;
  _totalSamples = 0;
}

void UniboardEmulator::getPacketData(char *&ptr, unsigned long &size)
{
  ptr = _packet.data();
  size = _packet.size();
}
