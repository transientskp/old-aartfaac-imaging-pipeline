#include "CalibratorTest.h"
#include "../modules/calibrator/Calibrator.h"
#include "../StreamBlob.h"
#include "../../Constants.h"

#include <pelican/utility/ConfigNode.h>
#include <eigen3/Eigen/Dense>

CPPUNIT_TEST_SUITE_REGISTRATION(CalibratorTest);


CalibratorTest::CalibratorTest():
  CppUnit::TestFixture(),
  mCalibrator(NULL),
  mStreamBlob(NULL)
{
}

void CalibratorTest::setUp()
{
  pelican::ConfigNode config(
    "<Calibrator>"
    "  <output path=\"/dev/null\"/>"
    "  <uvw path=\"/opt/aartfaac/uvw_merged.txt\"/>"
    "  <lba type=\"1\"/> <!-- 0 = LBA_INNER, 1 = LBA_OUTER -->"
    "</Calibrator>"
  );
  mCalibrator = new Calibrator(config);
  mStreamBlob = new StreamBlob();
  mStreamBlob->mXX = MatrixXcf::Random(NUM_ANTENNAS, NUM_ANTENNAS);
  mStreamBlob->mXX.array() + mStreamBlob->mXX.transpose().array();
  mStreamBlob->mMJDTime = 4.83793e+09;
  mStreamBlob->mFrequency = 5.97565e+07;
}

void CalibratorTest::tearDown()
{
  delete mCalibrator;
  delete mStreamBlob;
}

void CalibratorTest::calibrate()
{
  mCalibrator->run(mStreamBlob, mStreamBlob);
}

void CalibratorTest::gainSolve()
{
  const int n = 3;
  VectorXcf antennas = VectorXcf::Random(n) * 10.0f;
  MatrixXcf model = antennas * antennas.adjoint();
  for (int i = 0; i < n; i++)
  {
    model(i,i) = std::complex<float>(0.0f, 0.0f);
    for (int j = 0; j < n; j++)
      model(i,j).imag() = -model(i,j).imag();
  }

  VectorXcf gains(antennas);
  gains += VectorXcf::Random(n) * 2.0f;
  MatrixXcf data = gains.asDiagonal() * model * gains.asDiagonal().toDenseMatrix().adjoint();

  VectorXcf recov(n), initial_gains(n);
  for (int i = 0; i < n; i++)
    initial_gains(i) = std::complex<float>(1.0f, 1.0f);

  mCalibrator->gainSolv(model, data, initial_gains, recov);

  float org = ((gains.adjoint() * gains).array().abs()(0));
  float rec = ((recov.adjoint() * recov).array().abs()(0));

  CPPUNIT_ASSERT_DOUBLES_EQUAL(org, rec, 0.3);
}

void CalibratorTest::walsCalibration()
{
  MatrixXcf data(10,10);
  data <<
 std::complex<float>(1,0), std::complex<float>(0.06323387456779367,0.04564188611239615), std::complex<float>(-0.02068193272433947,-0.002480544766974331), std::complex<float>(-0.02297857114452227,0.01773178268753087), std::complex<float>(0.006876484288534357,-0.0117752877803715), std::complex<float>(-0.001407629854837838,0.01146962902660607), std::complex<float>(-0.01610723454884124,-0.00320976499812113), std::complex<float>(-0.01086123000672874,0.01583962022466099), std::complex<float>(0.008665102122104329,0.004872301272481037), std::complex<float>(0.01432119347631315,-0.002113171140435892),
 std::complex<float>(0.06323387456779367,-0.04564188611239615), std::complex<float>(1,0), std::complex<float>(0.01023751673907744,-0.03375128785715664), std::complex<float>(-0.02757422060150805,0.01205473817183712), std::complex<float>(0.00708900379671944,-0.004257465269677095), std::complex<float>(0.0210948235881623,-0.01508578069610451), std::complex<float>(-0.01049724448583607,-0.00627116511693507), std::complex<float>(-0.001168695800731785,0.03201935521420603), std::complex<float>(-0.0101655052559618,0.01495736997708248), std::complex<float>(-0.009072757948950412,0.009515953626591023),
 std::complex<float>(-0.02068193272433947,0.002480544766974331), std::complex<float>(0.01023751673907744,0.03375128785715664), std::complex<float>(1,0), std::complex<float>(-0.01980226521299289,-0.004642071992013171), std::complex<float>(0.01256707873226296,-0.04521404014357155), std::complex<float>(0.0009700136506740253,-0.007470386365754468), std::complex<float>(0.007963625653636015,-0.020410094154271), std::complex<float>(0.02080539055831511,-0.00756921451455402), std::complex<float>(0.01699133084733353,-0.007283776363866283), std::complex<float>(-0.02970539722863378,0.004837962223921299),
 std::complex<float>(-0.02297857114452227,-0.01773178268753087), std::complex<float>(-0.02757422060150805,-0.01205473817183712), std::complex<float>(-0.01980226521299289,0.004642071992013171), std::complex<float>(1,0), std::complex<float>(-0.06152434283917976,-0.001502411622720983), std::complex<float>(-0.04219814561416377,-0.04693388463321586), std::complex<float>(0.008357901198053491,-0.01449148654766701), std::complex<float>(0.005412819756633522,-0.01845752245288859), std::complex<float>(0.02732862290938642,0.008636758866947647), std::complex<float>(-0.03839155469747819,0.02734937409143568),
 std::complex<float>(0.006876484288534357,0.0117752877803715), std::complex<float>(0.00708900379671944,0.004257465269677095), std::complex<float>(0.01256707873226296,0.04521404014357155), std::complex<float>(-0.06152434283917976,0.001502411622720983), std::complex<float>(1,0), std::complex<float>(0.09757182871770051,-0.006968631896675892), std::complex<float>(0.02293234352457401,0.009471668937094368), std::complex<float>(0.0151169836966824,-0.005593506869884442), std::complex<float>(0.01779594623116046,-0.01129185887477924), std::complex<float>(-0.02758108921802683,-0.02210150393949873),
 std::complex<float>(-0.001407629854837838,-0.01146962902660607), std::complex<float>(0.0210948235881623,0.01508578069610451), std::complex<float>(0.0009700136506740253,0.007470386365754468), std::complex<float>(-0.04219814561416377,0.04693388463321586), std::complex<float>(0.09757182871770051,0.006968631896675892), std::complex<float>(1,0), std::complex<float>(0.007424182608098595,0.03917013228368867), std::complex<float>(-0.005289151859434949,0.0003659153461972407), std::complex<float>(-0.003323134911676914,-0.01622253512619533), std::complex<float>(0.02340192908674028,-0.01698309128916495),
 std::complex<float>(-0.01610723454884124,0.00320976499812113), std::complex<float>(-0.01049724448583607,0.00627116511693507), std::complex<float>(0.007963625653636015,0.020410094154271), std::complex<float>(0.008357901198053491,0.01449148654766701), std::complex<float>(0.02293234352457401,-0.009471668937094368), std::complex<float>(0.007424182608098595,-0.03917013228368867), std::complex<float>(1,0), std::complex<float>(-0.004455939204144937,-0.008829179261900253), std::complex<float>(0.02138788191219421,0.01258896025248566), std::complex<float>(-0.003341227745511196,-0.03102383079880248),
 std::complex<float>(-0.01086123000672874,-0.01583962022466099), std::complex<float>(-0.001168695800731785,-0.03201935521420603), std::complex<float>(0.02080539055831511,0.00756921451455402), std::complex<float>(0.005412819756633522,0.01845752245288859), std::complex<float>(0.0151169836966824,0.005593506869884442), std::complex<float>(-0.005289151859434949,-0.0003659153461972407), std::complex<float>(-0.004455939204144937,0.008829179261900253), std::complex<float>(1,0), std::complex<float>(0.01517146352788157,-0.005481072223582672), std::complex<float>(0.02083221916855265,0.001265463987593149),
 std::complex<float>(0.008665102122104329,-0.004872301272481037), std::complex<float>(-0.0101655052559618,-0.01495736997708248), std::complex<float>(0.01699133084733353,0.007283776363866283), std::complex<float>(0.02732862290938642,-0.008636758866947647), std::complex<float>(0.01779594623116046,0.01129185887477924), std::complex<float>(-0.003323134911676914,0.01622253512619533), std::complex<float>(0.02138788191219421,-0.01258896025248566), std::complex<float>(0.01517146352788157,0.005481072223582672), std::complex<float>(1,0), std::complex<float>(-0.02303173618565119,0.06334162044113673),
 std::complex<float>(0.01432119347631315,0.002113171140435892), std::complex<float>(-0.009072757948950412,-0.009515953626591023), std::complex<float>(-0.02970539722863378,-0.004837962223921299), std::complex<float>(-0.03839155469747819,-0.02734937409143568), std::complex<float>(-0.02758108921802683,0.02210150393949873), std::complex<float>(0.02340192908674028,0.01698309128916495), std::complex<float>(-0.003341227745511196,0.03102383079880248), std::complex<float>(0.02083221916855265,-0.001265463987593149), std::complex<float>(-0.02303173618565119,-0.06334162044113673), std::complex<float>(1,0);


  MatrixXcf model(10,3);
  model <<
 std::complex<float>(0.7698691961555693,-0.6382017085614685), std::complex<float>(-0.6031413164767803,-0.797634347529403), std::complex<float>(0.3626253235822198,-0.9319350163487208),
 std::complex<float>(0.8979142601904846,-0.4401704003526074), std::complex<float>(-0.8532105526312779,-0.5215666332105894), std::complex<float>(0.8036159866907532,-0.5951481714119999),
 std::complex<float>(0.0876176455297185,0.9961541789260479), std::complex<float>(-0.7470289418994659,0.6647915161647028), std::complex<float>(-0.5565205502070202,0.8308338445184305),
 std::complex<float>(-0.9369269370548485,0.3495252703611285), std::complex<float>(-0.6595732064804967,0.7516403297409181), std::complex<float>(-0.7390552929880118,-0.6736447683359561),
 std::complex<float>(0.04577186345047258,0.9989519190212667), std::complex<float>(-0.7242315009345734,-0.689556910670943), std::complex<float>(0.9588903267011257,-0.283776921822421),
 std::complex<float>(0.5905906729086757,0.8069712863995085), std::complex<float>(0.2524952380255556,-0.9675981370250865), std::complex<float>(0.3049893747928697,-0.9523557535204239),
 std::complex<float>(-0.8536229667897153,0.5208913807783007), std::complex<float>(-0.9808232058835383,-0.1948995608007834), std::complex<float>(0.8328273761559053,-0.553532800767281),
 std::complex<float>(-0.5786967722896077,-0.8155427920971344), std::complex<float>(-0.1490901169129245,0.9888236127029383), std::complex<float>(0.5561079745516846,-0.8311100532661261),
 std::complex<float>(-0.7867192471684046,0.6173109638867422), std::complex<float>(-0.691231231344002,0.722633644950645), std::complex<float>(-0.9363318909457369,0.3511162058321728),
 std::complex<float>(0.2802400898300006,-0.9599299412207504), std::complex<float>(0.7071558002081223,-0.7070577587665736), std::complex<float>(0.4353022145835264,-0.9002843895007163);

  VectorXf fluxes(3);
  fluxes << 1.0, 1.209084157449707, 0.0;

  MatrixXf mask(10,10);
  mask.setOnes();

  VectorXcf gains;
  VectorXf powers;
  MatrixXcf noisecov(10,10);
  mCalibrator->walsCalibration(model, data, fluxes, mask, gains, powers, noisecov);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(powers(0), 1.0, 1e-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(powers(1), 1.02116, 1e-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(powers(2), 0.89910, 1e-5);
}
