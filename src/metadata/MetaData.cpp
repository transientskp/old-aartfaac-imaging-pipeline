#include "MetaData.h"


MetaData::MetaData()
{
  QString table_name = QCoreApplication::arguments().at(1);
  QString out_name = QCoreApplication::arguments().at(2);
  mTableName = qPrintable(table_name);
  mOutName = qPrintable(out_name);
}

MetaData::~MetaData()
{
}

void MetaData::start()
{
  String command("select from " + mTableName +
    " where TIME = 0 giving " + mOutName + " as plain");

  std::cout << "QUERY: " << command.c_str() << std::endl;
  Table table = tableCommand(command);
  table.flush();
}
