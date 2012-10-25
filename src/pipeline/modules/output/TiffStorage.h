#ifndef TIFFSTORAGE_H
#define TIFFSTORAGE_H

#include <pelican/output/AbstractOutputStream.h>
#include <pelican/utility/ConfigNode.h>

#include <QtGui/QColor>
#include <QtCore>

using namespace pelican;

class TiffStorage : public AbstractOutputStream
{
public:
  /// Constructor
  TiffStorage(const ConfigNode &inConfigNode);

  /// Destructor
  ~TiffStorage();

protected:
  /// Send the datablob to the output stream
  void sendStream(const QString &inStreamName, const DataBlob *inDataBlob);

private:
  QString mPath; ///< Path to store images at
  QVector<QRgb> mColors; ///< Colortable for 8bit image
};

PELICAN_DECLARE(AbstractOutputStream, TiffStorage)

#endif // TIFFSTORAGE_H
