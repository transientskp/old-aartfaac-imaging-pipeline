#include "TiffStorage.h"
#include "../../StreamBlob.h"

#include <limits>
#include <cmath>
#include <QtGui/QImage>
#include <QtGui/QColor>

TiffStorage::TiffStorage(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("output", "path", "./");

  QColor color;

  for (int i = 0; i < 256; i++)
  {
    int hue = (int) round(240.0 - (i / 255.0 * 240.0));
    color.setHsv(hue, 255, 255);
    mColors.append(color.rgb());
  }
}

TiffStorage::~TiffStorage()
{

}

void TiffStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const StreamBlob *blob = static_cast<const StreamBlob *>(inDataBlob);

  if (blob->type() != "StreamBlob")
  {
    qWarning("Expected 'StreamBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

/*
  const std::vector<float> &skymap = blob->getSkyMap();

  // Obtain min and max value
  float min = std::numeric_limits<float>::max();

  float max = std::numeric_limits<float>::min();

  for (int i = 0, n = skymap.size(); i < n; i++)
  {
    min = std::min<float>(min, skymap[i]);
    max = std::max<float>(max, skymap[i]);
  }

  // Normalize between 0..1 and find gamma
  std::vector<float> normalized(skymap.size());
  float gamma = 0.75f;

  for (int i = 0, n = skymap.size(); i < n; i++)
    normalized[i] = (skymap[i] - min) / (max - min);

  // Apply gamma correction and put into bitmap
  // See http://en.wikipedia.org/wiki/Gamma_correction
  std::vector<unsigned char> bitmap(skymap.size());

  for (int i = 0, n = skymap.size(); i < n; i++)
    bitmap[i] = (unsigned char) round(std::pow<float>(normalized[i], gamma) * 255.0f);

  // Create image
  QImage image(&bitmap[0], blob->getWidth(), blob->getHeight(), QImage::Format_Indexed8);
  image.setColorTable(mColors);
  QString filename = QString::number(blob->getFrequency()) + "_" + blob->getDateTime().toString("dd-MM-yyyy_hh:mm:ss") + ".tiff";
  image.save(mPath + "/" + filename, "TIFF");
*/
}

