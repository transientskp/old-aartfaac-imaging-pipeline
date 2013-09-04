#include "FlaggerTest.h"
#include "../modules/flagger/Flagger.h"
#include "../StreamBlob.h"
#include "../../Constants.h"
#include "../../Macros.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(FlaggerTest);

FlaggerTest::FlaggerTest():
  CppUnit::TestFixture(),
  mFlagger(NULL),
  mStreamBlob(NULL)
{
}

void FlaggerTest::setUp()
{
  pelican::ConfigNode config(
    "<Flagger>"
    "   <deviation multiplier=\"4.0\"/>"
    "</Flagger>"
  );
  mFlagger = new Flagger(config);
  mStreamBlob = new StreamBlob();
  Data(mStreamBlob) = MatrixXcf::Random(NUM_ANTENNAS, NUM_ANTENNAS);
  Data(mStreamBlob).array() + Data(mStreamBlob).transpose().array();

  // Artificial increase antenna 10
  for (int i = 0; i < NUM_ANTENNAS; i++)
  {
    Data(mStreamBlob)(i,10) += 10.0f;
    Data(mStreamBlob)(10,i) += 10.0f;
  }
}

void FlaggerTest::tearDown()
{
  delete mFlagger;
  delete mStreamBlob;
}

void FlaggerTest::flag()
{
  mFlagger->run(0, mStreamBlob, mStreamBlob);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(Mask(mStreamBlob).col(10).sum(), NUM_ANTENNAS, 1e-5f);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(Mask(mStreamBlob).row(10).sum(), NUM_ANTENNAS, 1e-5f);
}

