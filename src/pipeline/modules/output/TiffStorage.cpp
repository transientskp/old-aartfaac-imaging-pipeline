#include "TiffStorage.h"
#include "../../StreamBlob.h"
#include "../../../utilities/Utils.h"
#include "../../../Constants.h"

#include <limits>
#include <cmath>
#include <QtGui/QColor>
#include <eigen3/Eigen/Dense>

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

  mImage = QImage(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE, QImage::Format_Indexed8);
  mImage.setColorTable(mColors);
}

TiffStorage::~TiffStorage()
{

}

void TiffStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const StreamBlob *blob = static_cast<const StreamBlob *>(inDataBlob);
  static int img_index = 0;

  if (blob->type() != "StreamBlob")
  {
    qWarning("Expected 'StreamBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

  Eigen::MatrixXf &skymap = const_cast<Eigen::MatrixXf&>(blob->mSkyMap);

  const float min = skymap.minCoeff();
  const float max = skymap.maxCoeff();
  const float gamma = 2.2f;

  skymap = (skymap.array() - min) / (max - min);

  QString filename = mPath + "/img-" + QString::number(img_index) + ".png";
  img_index++;

  for (int i = 0; i < skymap.size(); i++)
    *(mImage.bits() + i) = (uchar) roundf(std::pow(skymap(i), gamma) * 255.0f);

  if (!mImage.save(filename))
    qFatal("[%s] Could not save image to %s", __FUNCTION__, qPrintable(filename));
}

