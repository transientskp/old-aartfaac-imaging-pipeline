#include "TiffStorage.h"
#include "../../StreamBlob.h"

#include <limits>
#include <cmath>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <iostream>

TiffStorage::TiffStorage(const ConfigNode &inConfigNode)
  : AbstractOutputStream(inConfigNode)
{
  mPath = inConfigNode.getOption("file", "path", "./");

  QColor color;
  for (int i = 0; i < 256; i++)
  {
    int hue = (int) round(240.0 - (i/255.0*240.0));
    color.setHsv(hue, 255, 255);
    mColors.append(color.rgb());
  }
  // percent of pixel range at which to saturate,in order to show best contrast
  saturate_min = 0.0025; saturate_max = 0.12; 
  // saturate_min = 0.00; saturate_max = 0.30; // Good for 5min stretch
}

TiffStorage::~TiffStorage()
{

}

void TiffStorage::sendStream(const QString &inStreamName, const DataBlob *inDataBlob)
{
  const StreamBlob *blob = static_cast<const StreamBlob *>(inDataBlob);
  static int first = 1;
  static float caxis_min = 0, caxis_max = 0;

  if (blob->type() != "StreamBlob")
  {
    qWarning("Expected 'StreamBlob', got '%s' on stream '%s', ignoring...",
             qPrintable(blob->type()), qPrintable(inStreamName));
    return;
  }

  const std::vector<float> &skymap = blob->getSkyMap();
  const std::vector<float> &skymapradec = blob->getSkyMapradec();

  // Obtain min and max value
  float min = std::numeric_limits<float>::max();
  float max = std::numeric_limits<float>::min();
  for (int i = 0, n = skymap.size(); i < n; i++)
  { if (skymap[i] != 0)
    { min = std::min<float>(min, skymap[i]);
      max = std::max<float>(max, skymap[i]);
    }
  }
  std::cout <<" --> Pixel range in raw image:(" <<max<<", "<<min<<")\n";
  std::cerr <<" --> Pixel range in raw image:(" <<max<<", "<<min<<")\n";

  if (first)
  { first = 0;
    caxis_min = min + (max - min)*saturate_min; 
    caxis_max = min + (max - min)*saturate_max;
    std::cout<<"FIRST CALL ---> Caxis range: "<<caxis_min<<" "<<caxis_max;
  }

  // Normalize Skymap between 0..1 and find gamma
  std::vector<float> normalized(skymap.size());
  float gamma = 1; //0.75f;
  for (int i = 0, n = skymap.size(); i < n; i++)
    // normalized[i] = (skymap[i] - min) / (max - min);
    if (skymap[i] != 0)
    { if (skymap[i] < caxis_min) normalized[i] = 0; // caxis_min;
      else if (skymap[i] > caxis_max) normalized[i] = 1; // caxis_max;
      else normalized[i] = (skymap[i] - caxis_min) / (caxis_max - caxis_min);
    }
    else normalized[i] = 0;

  // Normalize Skymapradec between 0..1 and find gamma
  std::vector<float> normalized_radec(skymapradec.size());
  for (int i = 0, n = skymapradec.size(); i < n; i++)
    // normalized[i] = (skymap[i] - min) / (max - min);
    if (skymapradec[i] != 0)
    { if (skymapradec[i] < caxis_min) 
        normalized_radec[i] = 0; // caxis_min;
      else if (skymapradec[i] > caxis_max) 
        normalized_radec[i] = 1; // caxis_max;
      else normalized_radec[i] = 
        (skymapradec[i] - caxis_min) / (caxis_max - caxis_min);
    }
    else normalized_radec[i] = 0;

  // Apply gamma correction and put into bitmap
  // See http://en.wikipedia.org/wiki/Gamma_correction
  std::vector<unsigned char> bitmap(skymap.size());
  for (int i = 0, n = skymap.size(); i < n; i++)
    if (normalized[i] != 0)
      bitmap[i] = (unsigned char) 
                round(std::pow<float>(normalized[i], gamma) * 255.0f);
    else bitmap[i] = 0;

   // For the skymap in radec units
  std::vector<unsigned char> bitmap_radec(skymapradec.size());
  for (int i = 0, n = skymapradec.size(); i < n; i++)
    if (normalized_radec[i] != 0)
      bitmap_radec[i] = (unsigned char) 
                round(std::pow<float>(normalized_radec[i], gamma) * 255.0f);
    else bitmap_radec[i] = 0;

  // Create image
  QImage image(&bitmap[0], blob->getWidth(), blob->getHeight(), 
               QImage::Format_Indexed8);
  image.setColorTable(mColors);
  // QString filename = QString::number(blob->getFrequency(), 'f', 6) + "_" + blob->getDateTime().toString("dd-MM-yyyy_hh:mm:ss") + ".tiff";
  QString filename = QString::number(blob->getFrequency(), 'f', 0) + "_" + 
                     QString::number(blob->getMJDTime(), 'f', 0) + ".tiff";
  image.save(mPath + "/" + filename, "TIFF");

  // Create image
  QImage image_radec(&bitmap_radec[0], blob->getWidth(), blob->getHeight(), 
               QImage::Format_Indexed8);
  image_radec.setColorTable(mColors);
  // QString filename_radec = QString::number(blob->getFrequency(), 'f', 6) + "_"
  //                  + blob->getDateTime().toString("dd-MM-yyyy_hh:mm:ss") + 
  //                  "_radec" + ".tiff";
  QString filename_radec = QString::number(blob->getFrequency(), 'f', 0) + "_"
                         + QString::number(blob->getMJDTime(), 'f', 0) + "_radec" 
                         + ".tiff";
  image_radec.save(mPath + "/" + filename_radec, "TIFF");
}

