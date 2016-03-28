#!/usr/bin/env bash

export CXX=g++
export CC=gcc

INSTALL_DIR=/home/fhuizing/soft/release

if [ $1 ]
then
  INSTALL_DIR=$1
fi

git clean -xdf

cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCASACORE_ROOT=$INSTALL_DIR \
         -DPELICAN_INCLUDE_DIR=$INSTALL_DIR/include/pelican \
         -DPELICAN_LIBRARY=$INSTALL_DIR/lib/libpelican.so \
         -DPELICAN_TESTUTILS_LIBRARY=$INSTALL_DIR/lib/libpelican-testutils.so \
         -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
         -DEIGEN3_INCLUDE_DIR=$INSTALL_DIR/include/eigen3 \
         -DENABLE_OPENMP=OFF
