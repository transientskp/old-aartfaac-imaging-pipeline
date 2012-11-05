#include "MetaData.h"

#include <casacore/ms/MeasurementSets.h>

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

  qDebug("QUERY: %s", command.c_str());

  Table table = tableCommand(command);
  table.reopenRW();
  MeasurementSet ms(table);

  MSColumns msc(ms);
  ms.addRow();

  Array<Double> a(IPosition(1,3));
  Array<Double>::iterator i;
  for (i = a.begin(); i != a.end(); ++i)
    *i = 2.9;

  msc.uvw().put(0, a);
  msc.time().put(0, 1.03234);
  ms.flush();
}
