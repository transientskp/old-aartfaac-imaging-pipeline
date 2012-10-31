#include "MetaData.h"

using namespace std;

MetaData::MetaData()
{
  QString table_name = QCoreApplication::arguments().at(1);
  casa::Table table(qPrintable(table_name));
  mMeasurementSet = new casa::MeasurementSet(table);
  mMSColumns = new casa::ROMSColumns(*mMeasurementSet);
}

MetaData::~MetaData()
{
  delete mMeasurementSet;
  delete mMSColumns;
}

void MetaData::start()
{
  //casa::Array<casa::Double> data_array;
  //casa::Array<casa::Double>::iterator iter;

  // ANTENNA table
  int rows = mMSColumns->antenna().nrow();
  for (int i = 0; i < rows; i++)
  {
    cout << i << " ";
    cout << mMSColumns->antenna().name()(i).c_str();
    cout << endl;
  }
}
