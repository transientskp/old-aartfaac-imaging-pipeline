#include "UniboardEmulator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <pelican/utility/ConfigNode.h>


/**
 * Header definition:
 *
 * packet size    = sizeof(quint32) = 4 bytes
 * header size    = sizeof(quint32) = 4 bytes
 * nr of samples  = sizeof(quint32) = 4 bytes
 * sample size    = sizeof(quint32) = 4 bytes
 * channel id     = sizeof(quint32) = 4 bytes
 * packet counter = sizeof(quint64) = 8 bytes
 * reserved       = sizeof(quint32) = 4 bytes
 *                = 6 * 4 + 8       = 32 bytes
 */
#define HEADER_SIZE (6 * sizeof(quint32) + sizeof(quint64))

/**
 * Sample definition:
 *
 * time       = sizeof(double          = 8  bytes
 * antennae1  = sizeof(quint16)        = 2  bytes
 * antennae2  = sizeof(quint16)        = 2  bytes
 * data xx    = sizeof(complex<float>) = 8  bytes
 * data yy    = sizeof(complex<float>) = 8  bytes
 * data xy    = sizeof(complex<float>) = 8  bytes
 * data yx    = sizeof(complex<float>) = 8  bytes
 *            = 8 + 2*2 + 4*8          = 44 bytes
 */
#define SAMPLE_SIZE (sizeof(double) + 2*sizeof(quint16) + 4*sizeof(std::complex<float>))

UniboardEmulator::UniboardEmulator(const pelican::ConfigNode &inConfigNode)
  : AbstractUdpEmulator(inConfigNode)
{
  mTotalPackets = 0;
  mTotalSamples = 0;
  mRowIndex     = 0;

  mSamples = inConfigNode.getOption("packet", "samples", "52").toULong();
  mUdpPacket.resize(HEADER_SIZE + mSamples * SAMPLE_SIZE);

  QString table_name = QCoreApplication::arguments().at(1);
  mMeasurementSet = new casa::MeasurementSet(qPrintable(table_name));
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
  mTotalTableRows = mMSColumns->data().nrow();
  qDebug("Header size : %ld bytes", HEADER_SIZE);
  qDebug("Sample size : %ld bytes", SAMPLE_SIZE);
  qDebug("Packet size : %d bytes", mUdpPacket.size());
}

UniboardEmulator::~UniboardEmulator()
{
  qDebug("Packets sent: %lld", mTotalPackets);
  qDebug("Samples sent: %lld", mTotalSamples);
  qDebug("Bytes sent  : %lld", mUdpPacket.size() * mTotalPackets);
}

void UniboardEmulator::getPacketData(char *&outData, unsigned long &outSize)
{
  outData = mUdpPacket.data();
  outSize = mUdpPacket.size();

  qint32 channel_id = mMSColumns->spectralWindow().numChan()(0);
  quint64 max_rows = std::min<quint64>(mTotalTableRows, mRowIndex+mSamples);
  mTotalPackets++;
  mTotalSamples += max_rows - mRowIndex;

  // Set the packet header
  *reinterpret_cast<quint32*>(outData + 0)  = mUdpPacket.size();
  *reinterpret_cast<quint32*>(outData + 4)  = HEADER_SIZE;
  *reinterpret_cast<quint32*>(outData + 8)  = max_rows - mRowIndex;
  *reinterpret_cast<quint32*>(outData + 12) = SAMPLE_SIZE;
  *reinterpret_cast<quint32*>(outData + 16) = channel_id;
  *reinterpret_cast<quint64*>(outData + 20) = mTotalPackets;
  *reinterpret_cast<quint32*>(outData + 28) = 0;

  char *data = outData + HEADER_SIZE;

  // Set the packet data
  casa::Array<casa::Complex> data_array;
  casa::Array<casa::Complex>::iterator i;
  for (; mRowIndex < max_rows; mRowIndex++)
  {
    double time = mMSColumns->time()(mRowIndex);
    quint16 antenna1 = mMSColumns->antenna1()(mRowIndex);
    quint16 antenna2 = mMSColumns->antenna2()(mRowIndex);
    data_array = mMSColumns->data()(mRowIndex);

    // First 8 bytes used for time
    *reinterpret_cast<double*>(data) = time;
    data += sizeof(double);

    // Next 4 bytes used for antennae
    *reinterpret_cast<quint16*>(data) = antenna1;
    data += sizeof(quint16);

    *reinterpret_cast<quint16*>(data) = antenna2;
    data += sizeof(quint16);

    // Next 32 bytes used for data
    for (i = data_array.begin(); i != data_array.end(); i++)
    {
      *reinterpret_cast<float*>(data) = float((*i).real());
      data += sizeof(float);

      *reinterpret_cast<float*>(data) = float((*i).imag());
      data += sizeof(float);
    }
  }
  Q_ASSERT(data == mUdpPacket.end());

  /*
  qDebug("--------Packet(%lld)--------", mTotalPackets);
  data = outData + HEADER_SIZE;
  for (quint64 i = 0; i < mSamples; i++)
  {
    double time = *reinterpret_cast<double*>(data);       data += sizeof(double);
    quint16 antenna1 = *reinterpret_cast<quint16*>(data); data += sizeof(quint16);
    quint16 antenna2 = *reinterpret_cast<quint16*>(data); data += sizeof(quint16);
    qDebug("  Sample(%lld): time(%f) a1(%u) a2(%u)", i, time, antenna1, antenna2);
    for (int j = 0; j < 4; j++)
    {
      std::complex<float> c;
      c.real() = *reinterpret_cast<float*>(data);
      data += sizeof(float);
      c.imag() = *reinterpret_cast<float*>(data);
      data += sizeof(float);
      qDebug("    Data(%d): real(%0.5f) imag(%0.5f)", j, c.real(), c.imag());
    }
  }
  */
}

unsigned long UniboardEmulator::interval()
{
  return 0;
}

int UniboardEmulator::nPackets()
{
  return 100;//mTotalTableRows / mSamples + mTotalTableRows % mSamples;
}
