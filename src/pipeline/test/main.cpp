#include <fstream>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/portability/Stream.h>
#include <cppunit/TestResultCollector.h>

char *gTableName; ///< Location of the dataset

int main(int, char **)
{
  gTableName = const_cast<char*>("/opt/aartfaac/TEST.MS");

  CppUnit::OFileStream stream("pipeline.xml");
  CppUnit::TextUi::TestRunner runner;
  CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), stream);

  runner.setOutputter(outputter);
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  runner.run();

  return runner.result().testFailuresTotal();
}
