Aartfaac Imaging Pipeline
=========================

Build instructions
------------------
    mkdir -p build
    cd build
    rm -rf *
    cmake -DPELICAN_ROOT= -DCASACORE_ROOT= -DMATLAB_ROOT= -DLOFAR_ROOT= -DUSE_LOFARSTMAN=True ..
    make

Directory structure
-------------------
    .
    ├── build
    ├── cmake
    │   └── modules
    └── src
        ├── emulator
        ├── pipeline
        │   └── modules
        ├── server
        └── utilities

* _build_, standard directory for building with `cmake ..`
* _cmake_, contains cmake related stuff `version.h.in`
* _cmake/modules_, cmake's `findpackage` modules
* _src_, the sourcecode structured conforming the system overview
