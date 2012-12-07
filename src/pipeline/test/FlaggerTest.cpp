#include "FlaggerTest.h"
#include "../modules/flagger/Flagger.h"
#include "../StreamBlob.h"
#include "../../Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(FlaggerTest);

FlaggerTest::FlaggerTest():
  CppUnit::TestFixture(),
  mFlagger(NULL)
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
  mStreamBlob->mXX = MatrixXcf::Random(NUM_ANTENNAS, NUM_ANTENNAS);
  mStreamBlob->mXX.array() + mStreamBlob->mXX.transpose().array();

  // Artificial increase antenna 10
  for (int i = 0; i < NUM_ANTENNAS; i++)
  {
    mStreamBlob->mXX(i,10) += 10.0f;
    mStreamBlob->mXX(10,i) += 10.0f;
  }
}

void FlaggerTest::tearDown()
{
  delete mFlagger;
  delete mStreamBlob;
}

void FlaggerTest::flag()
{
  mFlagger->run(mStreamBlob, mStreamBlob);

  for (int i = 0; i < NUM_ANTENNAS; i++)
  {
    if (i == 10)
      CPPUNIT_ASSERT_EQUAL(mStreamBlob->mFlagged[i], 1);
    else
      CPPUNIT_ASSERT_EQUAL(mStreamBlob->mFlagged[i], 0);
  }
}

