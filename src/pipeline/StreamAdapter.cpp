#include "StreamAdapter.h"
#include "StreamBlob.h"
#include "../emulator/stream/StreamUdpPacket.h"

#include <complex>

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

      blob->addSample(correlation.a1, correlation.a2,
                      correlation.polarizations[0],
                      correlation.polarizations[1],
                      correlation.polarizations[2],
                      correlation.polarizations[3]
                      );
    }
  }
}
