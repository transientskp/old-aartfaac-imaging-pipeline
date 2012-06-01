#ifndef CASAIMAGESTORAGE_H
#define CASAIMAGESTORAGE_H

#include <pelican/output/AbstractOutputStream.h>
#include <pelican/utility/ConfigNode.h>

using namespace pelican;

class CasaImageStorage : public AbstractOutputStream
{
public:
  /// Constructor
  CasaImageStorage(const ConfigNode& inConfigNode);

  /// Destructor
  ~CasaImageStorage();

protected:
  /// Send the datablob to the output stream
  void sendStream(const QString &inStreamName, const DataBlob *inDataBlob);

private:
  QString mPath; ///< Path to store images at
};

PELICAN_DECLARE(AbstractOutputStream, CasaImageStorage)

#endif // CASAIMAGESTORAGE_H
