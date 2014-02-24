Aartfaac Imaging Pipeline
=========================

Build instructions
------------------
## Local machine
    cd build
    cmake -DPELICAN_ROOT= -DCASACORE_ROOT= ..
    make

## Das4 cluster
	cd build
	cmake .. -DQT_QMAKE_EXECUTABLE=/var/scratch/fhuizing/usr/local/bin/qmake -DPELICAN_INCLUDE_DIR=/var/scratch/fhuizing/usr/local/include/pelican -DPELICAN_LIBRARY=/var/scratch/fhuizing/usr/local/lib/libpelican.so -DPELICAN_TESTUTILS_LIBRARY=/var/scratch/fhuizing/usr/local/lib/libpelican-testutils.so -DCASACORE_ROOT=/var/scratch/swinbank/sw-20130410/ -DEIGEN3_INCLUDE_DIR=/var/scratch/fhuizing/usr/local/include/eigen3 -DENABLE_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
	make -j
