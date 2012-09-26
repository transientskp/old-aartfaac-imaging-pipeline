Aartfaac Imaging Pipeline
=========================

Build instructions
------------------
    mkdir -p build
    cd build
    rm -rf *
    cmake -DPELICAN_ROOT= -DCASACORE_ROOT= -DLOFAR_ROOT= -DUSE_LOFARSTMAN=True ..
    make
