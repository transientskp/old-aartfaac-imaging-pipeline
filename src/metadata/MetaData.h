#ifndef META_DATA_H
#define META_DATA_H

#include "casacore/tables/Tables/TableParse.h"
#include <QtCore>

using namespace casa;

class MetaData
{
public:
  MetaData();
  ~MetaData();

  void start();

private:
   String mTableName;
   String mOutName;
};

#endif // META_DATA_H
