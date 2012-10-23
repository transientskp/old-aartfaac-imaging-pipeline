#ifndef UNIBOARD_SERVICE_BLOB_H
#define UNIBOARD_SERVICE_BLOB_H

#include <pelican/data/DataBlob.h>

using namespace pelican;

class UniboardServiceBlob : public DataBlob
{
public:
    UniboardServiceBlob();
};

PELICAN_DECLARE_DATABLOB(UniboardServiceBlob)

#endif // UNIBOARD_SERVICE_BLOB_H
