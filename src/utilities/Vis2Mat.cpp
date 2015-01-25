#include <iostream>
#include <sstream>
#include <vector>
#include <eigen3/Eigen/Dense>

#include "Constants.h"
#include "../emulator/stream/StreamPacket.h"
#include "../utilities/Utils.h"

#define NUM_CHANNELS 63

using namespace std;

int main(int argc, char *argv[])
{
  StreamHeader hdr;
  MatrixXcf m[NUM_CHANNELS][NUM_POLARIZATIONS];
  stringstream ss;
  size_t n;
  std::complex<float> v;

  for (int c = 0; c < NUM_CHANNELS; c++)
    for (int p = 0; p < NUM_POLARIZATIONS; p++)
      m[c][p].resize(NUM_ANTENNAS, NUM_ANTENNAS);

  while (cin.peek() != EOF)
  {
    cin.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    if (hdr.magic != HEADER_MAGIC)
    {
      cerr << "Invalid magic" << endl;
      return 1;
    }

    n = 0;
    for (int a2 = 0; a2 < NUM_ANTENNAS; a2++)
    {
      for (int a1 = 0; a1 < (a2 + 1); a1++)
      {
        for (int c = 0; c < NUM_CHANNELS; c++)
        {
          for (int p = 0; p < NUM_POLARIZATIONS; p++)
          {
            cin.read(reinterpret_cast<char*>(&v), sizeof(v));
            m[c][p](a1,a2) = v;
            m[c][p](a2,a1) = conj(v);
            n += sizeof(v);
          }
        }
      }
    }

    cout << "Read " << n << " bytes" << endl;
    for (int c = 0; c < NUM_CHANNELS; c++)
    {
      ss.str(string());
      ss << "c" << c;
      utils::matrix2stderr(m[c][XX_POL], ss.str().c_str());
    }
    cout << "Written matrices to stderr" << endl;
    return 0;
  }

  return 0;
}
