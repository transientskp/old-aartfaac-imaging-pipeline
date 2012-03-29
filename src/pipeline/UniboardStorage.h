#ifndef UNIBOARD_STORAGE_H
#define UNIBOARD_STORAGE_H

#include <pelican/output/AbstractOutputStream.h>
#include <pelican/utility/ConfigNode.h>

#include <QtGui/QColor>
#include <QtCore>

using namespace pelican;

class UniboardStorage : public AbstractOutputStream
{
public:
  /// Constructor
  UniboardStorage(const ConfigNode& inConfigNode);

  /// Destructor
  ~UniboardStorage();

protected:
  /// Send the datablob to the output stream
  void sendStream(const QString &inStreamName, const DataBlob *inDataBlob);

private:
  QString mPath; ///< Path to store images at
  QVector<QRgb> mColors; ///< Colortable for 8bit image
};

PELICAN_DECLARE(AbstractOutputStream, UniboardStorage)

#endif // UNIBOARD_STORAGE_H
