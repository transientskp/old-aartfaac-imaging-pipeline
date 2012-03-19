#include "UniboardClient.h"
#include "UniboardChunker.h"
#include "UniboardAdapter.h"

UniboardClient::UniboardClient(const ConfigNode &inConfigNode,
                               const DataTypes &inTypes,
                               const Config *inConfig)
  : DirectStreamDataClient(inConfigNode, inTypes, inConfig)
{
  addChunker("UniboardDataBlob", "UniboardChunker");
}

UniboardClient::~UniboardClient()
{

}
