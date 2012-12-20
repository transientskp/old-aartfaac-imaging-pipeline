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

void UtilitiesTest::precession()
{
  Matrix3f M;
  utils::precessionMatrix<float>(0.0, M);
  Matrix3f A;
  A << -0.041394f, 0.907724f, 0.417520f,
       -0.912267f, 0.136094f,-0.386324f,
       -0.407498f,-0.396881f, 0.822454f;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(A(i,j), M(i,j), 1e-5);
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

void UtilitiesTest::sph2cart()
{
  VectorXf theta(3);
  theta << 0.0f, 0.5f, 1.0f;
  theta *= M_PI;

  VectorXf phi(3);
  phi << 0.0f, 0.0f, 0.0f;

  float radius = 1.0f;

  MatrixXf C(3,3), D(3,3);
  utils::spherical2cartesian<float>(theta, phi, radius, C);

  D <<  1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
       -1.0f, 0.0f, 0.0f;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(D(i,j), C(i,j), 1e-5);
}
