#!/bin/bash

set -e

git reset --hard
sh /usr/local/lofarinit.sh
cd build
sh clean.sh
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DENABLE_TESTS=ON
make -j5
make test
