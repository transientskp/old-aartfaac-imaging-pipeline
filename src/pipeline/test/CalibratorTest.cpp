#include "CalibratorTest.h"
#include "../modules/calibrator/Calibrator.h"
#include "../StreamBlob.h"
#include "../../Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(CalibratorTest);


CalibratorTest::CalibratorTest():
  CppUnit::TestFixture(),
  mCalibrator(NULL)
{
}

void CalibratorTest::setUp()
{
  pelican::ConfigNode config(
    "<Calibrator>"
    "  <output path=\"/dev/null\"/>"
    "  <uvw path=\"/opt/aartfaac/uvw_merged.txt\"/>"
    "  <lba type=\"1\"/> <!-- 0 = LBA_INNER, 1 = LBA_OUTER -->"
    "</Calibrator>"
  );
  mCalibrator = new Calibrator(config);
}

void CalibratorTest::tearDown()
{
  delete mCalibrator;
}

void CalibratorTest::gainSolve()
{
}

void CalibratorTest::stefCal()
{
}
