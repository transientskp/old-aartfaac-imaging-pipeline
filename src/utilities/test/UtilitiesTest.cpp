#include "UtilitiesTest.h"
#include "../Utils.h"
#include "../NMSMax.h"
#include "Constants.h"
#include <complex>
#include <cmath>
#include <vector>

using namespace utils;

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

double f(const VectorXd &x)
{
  return pow( pow(x[0],2) + pow(x[1],2) + pow(x[2],2), 2 ) - pow(x[0]-3*x[1],2) + pow(x[2]-2, 2);
}

void UtilitiesTest::subband2frequency()
{
  float clock = 2e8f;
  float sb_bw = clock / 1024.0f;
  int subband = 296;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(Subband2Frequency(subband, clock), 57.8125e6f, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(Channel2Frequency(subband, MAX_MERGE_CHANNELS/2, clock), 57.8125e6f, 1e-6);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(Range2Frequency(subband, 0, MAX_MERGE_CHANNELS, clock), 57.8125e6f, 1e-6);
  float bw = Channel2Frequency(subband, MAX_MERGE_CHANNELS, clock) - Channel2Frequency(subband, 0, clock);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(bw, sb_bw, 1e-6);
}

void UtilitiesTest::simplex()
{
  VectorXd init(3), output(3);
  init << 1.23, 10.96, 1.0;

  using NM::Simplex;
  int count;
  output = Simplex(f, init, count);

  double d_x0, d_x1, d_x2;

  // df/dx0
  d_x0 = 2.0*(pow(output(0), 2.0)+pow(output(1), 2.0)+pow(output(2), 2.0));
  d_x0 *= 2.0*output(0);
  d_x0 -= 2.0*(output(0)-3.0*output(1));
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d_x0, 1e-3);

  // df/dx1
  d_x1 = 2.0*(pow(output(0), 2.0)+pow(output(1), 2.0)+pow(output(2), 2.0));
  d_x1 *= 2.0*output(1);
  d_x1 += 6.0*(output[0]-3.0*output(1));
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, d_x1, 1e-3);

  // df/dx2
  d_x2 = 2.0*(pow(output(0), 2.0)+pow(output(1), 2.0)+pow(output(2), 2.0));
  d_x2 *= 2.0*output(2);
  d_x2 += 2.0*(output(2)-2.0);
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
  Matrix3d M;
  utils::precessionMatrix<double>(0.0, M);
  Matrix3d A;
  A << -0.041394f, 0.907724f, 0.417520f,
       -0.912267f, 0.136094f,-0.386324f,
       -0.407498f,-0.396881f, 0.822454f;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(A(i,j), M(i,j), 1e-6);
}

void UtilitiesTest::radec2itrf()
{
  VectorXd ra(4);
  ra << 1.0f, 2.0f, 3.0f, 4.0f;
  VectorXd dec(4);
  dec << 2.0f, 3.0f, 4.0f, 5.0f;
  VectorXi epoch(4);
  epoch << 1, 0, 1, 0;

  double time = 70415.71996231399;

  MatrixXd itrf(4,3);
  utils::radec2itrf<double>(ra, dec, epoch, time, itrf);

  MatrixXd A(4,3);
  A << -0.1544349016871255, -0.1255864604850933, 0.9799887254880607,
       -0.8299218264791488, 0.4764017192305966, 0.2902949600796489,
       -0.4550525241190757, 0.2320855566768256, -0.8596880217118683,
       0.2211356581631649, 0.7362197855203525, -0.6395931895330891;

  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 3; j++)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(A(i,j), itrf(i,j), 1e-11);
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
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, ans, 1e-6);
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
      CPPUNIT_ASSERT_DOUBLES_EQUAL(D(i,j), C(i,j), 1e-6);
}
