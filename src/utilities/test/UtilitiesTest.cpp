#include "UtilitiesTest.h"
#include "../Utils.h"
#include "../Simplex.h"
#include <complex>
#include <cmath>
#include <vector>

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

double f(std::vector<double> &x)
{
  return pow( pow(x[0],2) + pow(x[1],2) + pow(x[2],2), 2 ) - pow(x[0]-3*x[1],2) + pow(x[2]-2, 2);
}

void UtilitiesTest::simplex()
{
  std::vector<double> init, output;
  init.push_back(1.23);
  init.push_back(10.96);
  init.push_back(1.0);

  using BT::Simplex;
  output = Simplex(f, init);

  CPPUNIT_ASSERT_EQUAL(3ul, output.size());

  double d_x0, d_x1, d_x2;

  // df/dx0
  d_x0 = 2.0*(pow(output[0], 2.0)+pow(output[1], 2.0)+pow(output[2], 2.0));
  d_x0 *= 2.0*output[0];
  d_x0 -= 2.0*(output[0]-3.0*output[1]);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d_x0, 1e-3);

  // df/dx1
  d_x1 = 2.0*(pow(output[0], 2.0)+pow(output[1], 2.0)+pow(output[2], 2.0));
  d_x1 *= 2.0*output[1];
  d_x1 += 6.0*(output[0]-3.0*output[1]);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d_x1, 1e-3);

  // df/dx2
  d_x2 = 2.0*(pow(output[0], 2.0)+pow(output[1], 2.0)+pow(output[2], 2.0));
  d_x2 *= 2.0*output[2];
  d_x2 += 2.0*(output[2]-2.0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d_x2, 1e-3);
}

void UtilitiesTest::pseudoinv()
{
  {
  MatrixXf M = MatrixXf::Random(3,2);
  MatrixXf I(3,2);

  utils::pseudoInverse<float>(M, I);

  MatrixXf eye = M.transpose() * I.transpose();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(0,0), 1.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(1,1), 1.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(0,1), 0.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(1,0), 0.0, 1e-6);
  }

  {
  MatrixXcf M = MatrixXcf::Random(3,2);
  MatrixXcf I(3,2);

  utils::pseudoInverse<std::complex<float> >(M, I);

  MatrixXcf eye = M.transpose() * I.transpose();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(0,0).real(), 1.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(1,1).real(), 1.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(0,1).real(), 0.0, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(eye(1,0).real(), 0.0, 1e-6);
  }

  {
  MatrixXf M = MatrixXf::Random(3,1);
  MatrixXf I(3,1);

  utils::pseudoInverse<float>(M, I);
  CPPUNIT_ASSERT_DOUBLES_EQUAL((I.array() * M.array()).sum(), 1.0, 1e-6);
  }
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

void UtilitiesTest::radec2itrf()
{
  VectorXf ra(4);
  ra << 1.0f, 2.0f, 3.0f, 4.0f;
  VectorXf dec(4);
  dec << 2.0f, 3.0f, 4.0f, 5.0f;
  VectorXi epoch(4);
  epoch << 1, 0, 1, 0;

  MatrixXf itrf(4,3);
  utils::radec2itrf<float>(ra, dec, epoch, 0.0, itrf);

  MatrixXf A(4,3);
  A <<  0.14208f,  0.14783f,  0.97875f,
        0.84192f, -0.44482f,  0.30545f,
        0.46940f, -0.23295f, -0.85170f,
       -0.19254f, -0.75409f, -0.62791f;

  // NOTE: c++'s cos() gives other values then octave's cos()
  //       therefore, precision of assert is lowered.
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(A(i,j), itrf(i,j), 1e-2);
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
