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
  utils::kronecker<MatrixXi>(A, B, C);

  D << 1,  3,  2,  6,
       2,  4,  4,  8,
       3,  9,  4, 12,
       6, 12,  8, 16;

  CPPUNIT_ASSERT_EQUAL(C, D);
}

void UtilitiesTest::khatrirao()
{
  MatrixXi A(2,2), B(2,2), C(4,2), D(4,2);
  A << 1, 2,
       3, 4;

  B = A.transpose();
  utils::khatrirao<MatrixXi>(A, B, C);

  D << 1,  6,
       2,  8,
       3, 12,
       6, 16;

  CPPUNIT_ASSERT_EQUAL(C, D);
}
