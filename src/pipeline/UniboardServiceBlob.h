#ifndef UNIBOARD_SERVICE_BLOB_H
#define UNIBOARD_SERVICE_BLOB_H

#include <pelican/data/DataBlob.h>

using namespace pelican;

class ServiceBlob : public DataBlob
{
public:
    ServiceBlob();
};

PELICAN_DECLARE_DATABLOB(ServiceBlob)

#endif // UNIBOARD_SERVICE_BLOB_H
