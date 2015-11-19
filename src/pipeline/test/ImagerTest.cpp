#include "ImagerTest.h"
#include "../modules/imager/Imager.h"
#include "../StreamBlob.h"
#include "Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(ImagerTest);

ImagerTest::ImagerTest():
  CppUnit::TestFixture(),
  mImager(NULL)
{
}

void ImagerTest::setUp()
{
  pelican::ConfigNode config(
    "<Imager>"
    "  <output path=\"/dev/null\"/>"
    "  <uvw path=\"/opt/aartfaac/uvw_merged.txt\"/>"
    "  <lba type=\"1\"/> <!-- 0 = LBA_INNER, 1 = LBA_OUTER -->"
    "</Imager>"
  );
  mImager = new Imager(config);
}

void ImagerTest::tearDown()
{
  delete mImager;
}

void ImagerTest::fftShift()
{
  Eigen::MatrixXcf A(2,2), B(2,2);
  A << std::complex<float>(1.0f, 1.0f),
       std::complex<float>(2.0f, 2.0f),
       std::complex<float>(3.0f, 3.0f),
       std::complex<float>(4.0f, 4.0f);

  B = A;
  mImager->fftShift(B);
  CPPUNIT_ASSERT_EQUAL(A(0,0), B(1,1));
  CPPUNIT_ASSERT_EQUAL(A(0,1), B(1,0));
}

void ImagerTest::gridding()
{
  Eigen::MatrixXf mask(NUM_ANTENNAS, NUM_ANTENNAS);
  mask.setZero();
  Eigen::MatrixXcf acc = Eigen::MatrixXcf::Random(NUM_ANTENNAS, NUM_ANTENNAS);
  acc.array() += acc.transpose().array();
  Eigen::MatrixXcf grid(IMAGE_OUTPUT_SIZE, IMAGE_OUTPUT_SIZE);
  Eigen::MatrixXf X = Eigen::MatrixXf::Random(NUM_ANTENNAS, NUM_ANTENNAS).array() * IMAGE_OUTPUT_SIZE/2-1;
  Eigen::MatrixXf Y = X.transpose();

  mImager->gridding(acc, X, Y, mask, grid);

  std::complex<float> c_sum = acc.sum();
  std::complex<float> g_sum = grid.sum();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(c_sum.real(), g_sum.real(), 4.0f);
}
