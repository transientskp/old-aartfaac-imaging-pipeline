#ifndef UNIBOARD_EMULATOR_H
#define UNIBOARD_EMULATOR_H

#include <pelican/emulator/AbstractUdpEmulator.h>
#include <QtCore/QByteArray>

class UniboardEmulator : public pelican::AbstractUdpEmulator
{
public:
  UniboardEmulator(const pelican::ConfigNode &configNode);

  ~UniboardEmulator() {}

  void getPacketData(char*& ptr, unsigned long& size);

private:
  unsigned long _counter;
  unsigned long long _totalSamples;
  unsigned long _samples;
  QByteArray _packet;
};

#endif // UNIBOARD_EMULATOR_H
