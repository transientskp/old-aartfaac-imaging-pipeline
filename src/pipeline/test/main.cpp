#include <fstream>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/portability/Stream.h>

int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;

	CppUnit::OFileStream stream("pipeline.xml");
  CppUnit::TextUi::TestRunner runner;
	CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), stream);

	runner.setOutputter(outputter);
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
  return runner.run() ? 1 : 0;
}
