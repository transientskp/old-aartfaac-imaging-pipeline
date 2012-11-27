#include "Imager.h"

#include "../../StreamBlob.h"
#include "../../../matlabbridge/src/cpp/MatlabBridge.h"

#include <pelican/utility/Config.h>
#include <QtCore>

Imager::Imager(const ConfigNode &inConfig)
  : AbstractModule(inConfig)
{
  mULoc.resize(288 * 288);
  mVLoc.resize(288 * 288);
  QString uloc_filename = inConfig.getOption("uloc", "filename");
  QString vloc_filename = inConfig.getOption("vloc", "filename");
  readData(uloc_filename, mULoc);
  readData(vloc_filename, mVLoc);
  mBridge = MatlabBridge::singleton();
}

Imager::~Imager()
{
  //delete mBridge;
}

void Imager::readData(const QString &inFilename, std::vector<float> &outData)
{
  QFile file(inFilename);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qCritical("Failed opening %s", qPrintable(inFilename));
    return;
  }

  quint32 i = 0;
  QTextStream txt(&file);
  while (!txt.atEnd()) {
    QString line = txt.readLine();
    outData[i] = line.toFloat();
    i++;
  }
  qDebug("parsed %s", qPrintable(inFilename));
  Q_ASSERT(i == 288*288);
}

void Imager::run(const StreamBlob *input, StreamBlob *output)
{
  output->setMJDTime(input->getMJDTime());
  const std::vector<float> *real = input->getXXReal();
  const std::vector<float> *imag = input->getXXImag();

  std::vector<float> &skymap = output->getSkyMap();
  std::vector<float> &skymapradec = output->getSkyMapradec();
  std::vector<float> &vismap = output->getVisMap();
  mBridge->createImage(*real, *imag, input->getMJDTime(), input->getFrequency(),
                       mULoc, mVLoc, skymap, skymapradec, vismap);
}
