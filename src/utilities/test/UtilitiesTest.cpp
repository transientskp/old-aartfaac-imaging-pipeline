#include "UtilitiesTest.h"
#include "../Utils.h"


CPPUNIT_TEST_SUITE_REGISTRATION(UtilitiesTest);


UtilitiesTest::UtilitiesTest():
  CppUnit::TestFixture()
{
}

void UtilitiesTest::setUp()
{
}

void UtilitiesTest::tearDown()
{
}

void UtilitiesTest::dates()
{
}

void UtilitiesTest::kronecker()
{
  MatrixXi A(2,2), B(2,2), C(4,4), D(4,4);
  A << 1, 2,
       3, 4;

  B = A.transpose();
  utils::kronecker<int>(A, B, C);

  D << 1,  3,  2,  6,
       2,  4,  4,  8,
       3,  9,  4, 12,
       6, 12,  8, 16;

  CPPUNIT_ASSERT_EQUAL(D, C);
}

void UtilitiesTest::khatrirao()
{
  MatrixXi A(2,2), B(2,2), C(4,2), D(4,2);
  A << 1, 2,
       3, 4;

  B = A.transpose();
  utils::khatrirao<int>(A, B, C);

  D << 1,  6,
       2,  8,
       3, 12,
       6, 16;

  CPPUNIT_ASSERT_EQUAL(D, C);
}

void UtilitiesTest::polyval()
{
  {
  VectorXi P(6);
  P << 1, 2, 3, 4, 5, 6;
  int ans = utils::polyval<int>(P, 3);
  CPPUNIT_ASSERT_EQUAL(543, ans);
  }

  {
  VectorXd P(3);
  P << 0.0, 1.0, 0.0;
  double ans = utils::polyval<double>(P, 5.0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, ans, 1e-5);
  }
}
