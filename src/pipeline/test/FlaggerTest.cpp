#include "FlaggerTest.h"
#include "../modules/flagger/Flagger.h"
#include "../StreamBlob.h"
#include "Constants.h"
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
    "   <antenna sigma=\"3\" flagged=\"\"/>"
    "   <visibility sigma=\"3\" />"
    "</Flagger>"
  );
  mFlagger = new Flagger(config);
  mStreamBlob = new StreamBlob();
  mStreamBlob->mHeader.start_chan = 0;
  mStreamBlob->mHeader.end_chan = 62;
  mStreamBlob->reset();

  mStreamBlob->mRawData[0] = MatrixXcf::Random(mStreamBlob->mNumChannels, NUM_BASELINES);

  // Artificial increase certain visibilities
  mStreamBlob->mRawData[0](3, 2) = std::complex<float>(5.0f, 5.0f);
  mStreamBlob->mRawData[0](8, 7) = std::complex<float>(5.0f, 5.0f);
}

void FlaggerTest::tearDown()
{
  delete mFlagger;
  delete mStreamBlob;
}

void FlaggerTest::flag()
{
  mFlagger->run(0, mStreamBlob, mStreamBlob);
  CPPUNIT_ASSERT_EQUAL(2, int(mFlagger->mMask.size() - mFlagger->mMask.sum()));
}

