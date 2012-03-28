#include "UniboardStorage.h"
#include "UniboardDataBlob.h"

#include <limits>
#include <cmath>
#include <QtCore>
#include <QtGui/QImage>
#include <QtGui/QColor>

UniboardStorage::UniboardStorage(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("file", "path", "./");
  qDebug() << mPath;
}

UniboardStorage::~UniboardStorage()
{

}

void UniboardStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  Q_UNUSED(inStreamName);

  const UniboardDataBlob *blob = static_cast<const UniboardDataBlob*>(inDataBlob);

//  if (blob->type() != "image")
//  {
//    qWarning("Expected datablob type 'UniboardDataBlob', got '%s' ignoring...", qPrintable(blob->type()));
//    return;
//  }

  const std::vector<float> &skymap = blob->getSkyMap();

  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();
  //float val = 0.0f;

  //for (int i = 0, n = skymap.size(); i < n; i++)
  //{
  //  val = log(skymap[i]);
  //}
  min = log(90.0);;
  max = log(400.0);

  std::vector<unsigned char> normalized_skymap(skymap.size());
  for (int i = 0, n = skymap.size(); i < n; i++)
  {
    normalized_skymap[i] = (unsigned char) round(((log(skymap[i]) - min) / (max - min)) * 255.0f);
  }

  static QVector<QRgb> colors;
  if (colors.empty())
  {
    QColor color;
    for (int i = 0; i < 256; i++)
    {
      int hue = (int) round(240.0 - (i/255.0*240.0));
      color.setHsv(hue, 255, 255);
      colors.append(color.rgb());
    }
  }

  QImage image(&normalized_skymap[0], 512, 512, QImage::Format_Indexed8);
  image.setColorTable(colors);
  QString filename = blob->getDateTime().toString("_dd-MM-yyyy_hh-mm-ss") + ".tiff";
  image.save(mPath + "/" + filename, "TIFF");
}

