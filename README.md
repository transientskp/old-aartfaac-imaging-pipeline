Aartfaac Imaging Pipeline
=========================

System overview
---------------
###Emulator###

###Server###

###Pipeline###

####Modules####
* Rejector
* Calibrator
* Imager
* TransientDetector

Build instructions
------------------

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
