#ifndef DATA_CLIENT_H
#define DATA_CLIENT_H

#include <pelican/core/DirectStreamDataClient.h>

using namespace pelican;

class UniboardClient : public DirectStreamDataClient
{
public:
    UniboardClient(const ConfigNode& inConfigNode,
               const DataTypes &inTypes,
               const Config *inConfig);

    ~UniboardClient();
};

PELICAN_DECLARE_CLIENT(UniboardClient)

#endif // DATA_CLIENT_H
