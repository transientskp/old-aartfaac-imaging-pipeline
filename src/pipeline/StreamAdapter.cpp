#include "StreamAdapter.h"
#include "StreamBlob.h"
#include "../emulator/stream/StreamUdpPacket.h"

#include <complex>

extern "C" void halfp2singles(void *target, void *source, int numel);

// Construct the example adapter.
StreamAdapter::StreamAdapter(const ConfigNode &config)
  : AbstractStreamAdapter(config)
{
  mMaxPacketSamples = MAX_CORRELATIONS;
  mAntennae = config.getOption("antennae", "amount").toUInt();
  mPacketSize = sizeof(StreamUdpPacket);
}

void StreamAdapter::deserialise(QIODevice *inDevice)
{
  StreamBlob *blob = static_cast<StreamBlob *>(dataBlob());
  blob->reset();
  Q_ASSERT(chunkSize() % mPacketSize == 0);
  quint32 num_packets = chunkSize() / mPacketSize;

  StreamUdpPacket packet;
  quint64 bytes_read = 0;
  std::complex<float> polarizations[4];
  bool is_touched = false;

  for (quint32 i = 0; i < num_packets; i++)
  {
    while (inDevice->bytesAvailable() < mPacketSize)
      inDevice->waitForReadyRead(100);

    bytes_read += inDevice->read(reinterpret_cast<char *>(&packet), mPacketSize);

    if (!is_touched)
    {
      blob->setMJDTime(packet.mHeader.time);
      blob->mFrequency = packet.mHeader.freq;
      is_touched = true;
    }

    for (quint32 j = 0; j < packet.mHeader.correlations; j++)
    {
      StreamUdpPacket::Correlation &correlation = packet.mCorrelations[j];

      for (quint32 k = 0; k < 8; k += 2)
      {
        halfp2singles(static_cast<float *>(&polarizations[k / 2].real()),
                      static_cast<void *>(&correlation.polarizations[k]), 1);
        halfp2singles(static_cast<float *>(&polarizations[k / 2].imag()),
                      static_cast<void *>(&correlation.polarizations[k + 1]), 1);
      }

      blob->addSample(correlation.a1, correlation.a2,
                      polarizations[0],
                      polarizations[1],
                      polarizations[2],
                      polarizations[3]);
    }
  }
}