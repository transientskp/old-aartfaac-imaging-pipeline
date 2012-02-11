#include "UniboardEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

#define HEADER_SIZE 32

// packet row size:
// [u,v,w]                                       = 3x double
// [(Nc1, Nf1),(Nc2, Nf2),(Nc3, Nf3),(Nc4, Nf4)] = 8x double
// time                                          = 1x double
//                                               = 12 doubles
#define SAMPLE_SIZE 12 // sample size in doubles (one table row)

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &configNode)
  : AbstractUdpEmulator(configNode)
{
  _totalPackets = 0;
  _totalSamples = 0;

  _samples = configNode.getOption("packet", "samples").toULong();
  _packet.resize(_samples * SAMPLE_SIZE * sizeof(double) + HEADER_SIZE);

  QString tableName = QCoreApplication::arguments().at(1);
  _table = casa::Table::openTable(tableName.toStdString().c_str());
  _tableRows = _table.nrow();
  _timeColumn.attach(_table, "TIME_CENTROID");
  _dataColumn.attach(_table, "DATA");
  _posColumn.attach(_table, "UVW");

  char *ptr = _packet.data();
  *reinterpret_cast<int*>(ptr + 0) = _packet.size();
  *reinterpret_cast<int*>(ptr + 4) = HEADER_SIZE;
  *reinterpret_cast<int*>(ptr + 8) = _samples;
  *reinterpret_cast<int*>(ptr + 12) = SAMPLE_SIZE * sizeof(double);
}

void UniboardEmulator::getPacketData(char *&ptr, unsigned long &size)
{
  ptr = _packet.data();
  size = _packet.size();

  *reinterpret_cast<long int*>(ptr + 16) = _totalPackets;
  char *data = ptr + 32;

  double time;
  casa::Array<double> position;
  casa::Array<casa::Complex> coefficients;
  casa::Array<double>::iterator p;
  casa::Array<casa::Complex>::iterator c;

  unsigned dataIndex = 0;
  for (unsigned long i = 0; i < _samples; i++)
  {
    unsigned long long index = (i + _totalSamples) % _tableRows;
    time = _timeColumn.get(index);
    position = _posColumn.get(index);
    coefficients = _dataColumn.get(index);

    // First 1*8 bytes used for time
    reinterpret_cast<double*>(data)[dataIndex] = time;

    // Next 3*8 bytes used for pos
    for (p = position.begin(); p != position.end(); p++)
    {
      reinterpret_cast<double*>(data)[++dataIndex] = *p;
    }

    // Next 8*8 bytes used for complex numbers
    for (c = coefficients.begin(); c != coefficients.end(); c++)
    {
      reinterpret_cast<double*>(data)[++dataIndex] = (*c).real();
      reinterpret_cast<double*>(data)[++dataIndex] = (*c).imag();
    }

    dataIndex++;
    Q_ASSERT(dataIndex == i*SAMPLE_SIZE+12);
  }

  _totalPackets++;
  _totalSamples += _samples;
}
