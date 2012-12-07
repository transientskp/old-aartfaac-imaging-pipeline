#include "ImagerTest.h"
#include "../modules/imager/Imager.h"
#include "../StreamBlob.h"
#include "../../Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(ImagerTest);

char *gTableName;

ImagerTest::ImagerTest():
  CppUnit::TestFixture(),
  mImager(NULL)
{
}

void ImagerTest::setUp()
{
  gTableName = const_cast<char*>("/opt/aartfaac/TEST.MS");
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
  std::vector<int> flagged(2, 0);
  Eigen::MatrixXcf C(2, 2);
  Eigen::MatrixXcf G(4, 4);
  Eigen::MatrixXf X(2, 2), Y(2, 2);

  C << std::complex<float>(1.0f, 1.0f),
       std::complex<float>(2.0f, 2.0f),
       std::complex<float>(1.0f, 1.0f),
       std::complex<float>(2.0f, 2.0f);

  X << 0.2f, 0.3f,
       1.2f, 1.3f;

  Y = X.transpose();

  mImager->gridding(C, X, Y, flagged, G);

  std::complex<float> c_sum = C.sum();
  std::complex<float> g_sum = G.sum();

  CPPUNIT_ASSERT_EQUAL(c_sum, g_sum);
}
