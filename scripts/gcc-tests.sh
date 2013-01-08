#!/bin/bash

set -e

cd build
sh clean.sh
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DENABLE_TESTS=ON -DCASACORE_ROOT=/usr/local/casacore -DLOFAR_ROOT=/usr/local/LofIm
make -j5
make test
