#include "UniboardEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>

/**
 * packet row size:
 * (Nc1, Nf1) = 2 x sizeof(float)  = 8
 * time       = 1 x sizeof(double) = 8
 * one chunk (correlation matrix) would be (288*289/2) * 16 bytes = 41616 * 16 bytes
 * frequencies are sent from low to high
 */
#define SAMPLE_SIZE (2 * sizeof(float) + sizeof(double)) // sample size in bytes (one table row)

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode)
{
  mTotalPackets = 0;
  mTotalSamples = 0;

  mSamples = inConfigNode.getOption("packet", "samples").toULong();
  mUdpPacket.resize(mSamples * SAMPLE_SIZE);

  QString table_name = QCoreApplication::arguments().at(1);
  mTable = casa::Table::openTable(table_name.toStdString().c_str());
  mTotalTableRows = mTable.nrow();
  mTimeColumn.attach(mTable, "TIME");
  mDataColumn.attach(mTable, "DATA");
}

void UniboardEmulator::getPacketData(char *&outData, unsigned long &outSize)
{
  outData = mUdpPacket.data();
  outSize = mUdpPacket.size();

  quint32 data_index = 0;
  casa::Array<casa::Complex> data_array;

  for (quint32 i = 0; i < mSamples; i++)
  {
    quint64 row_index = (i + mTotalSamples) % mTotalTableRows;
    double time = mTimeColumn.get(row_index);
    data_array = mDataColumn.get(row_index);
    casa::Complex &xx_polar = (*data_array.begin());

    // First 8 bytes used for time
    reinterpret_cast<double*>(outData)[data_index++] = time;

    // Next 8 bytes used for first complex number (xx polarization)
    reinterpret_cast<float*>(outData)[data_index++] = float(xx_polar.real());
    reinterpret_cast<float*>(outData)[data_index++] = float(xx_polar.imag());
  }

  mTotalPackets++;
  mTotalSamples += mSamples;
}
