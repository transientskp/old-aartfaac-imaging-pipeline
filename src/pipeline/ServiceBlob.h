#ifndef SERVICE_BLOB_H
#define SERVICE_BLOB_H

#include <pelican/data/DataBlob.h>

using namespace pelican;

class ServiceBlob : public DataBlob
{
public:
    ServiceBlob();
};

PELICAN_DECLARE_DATABLOB(ServiceBlob)

#endif // SERVICE_BLOB_H
