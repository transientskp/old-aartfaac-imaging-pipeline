#!/bin/sh

export MATLAB_VERSION=matlab/64/2011a

module load $MATLAB_VERSION

export MATLAB_ROOT=/sara/sw/$VERSION
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/runtime/glnxa64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/os/glnxa64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/native_threads
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/server
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64
export XAPPLRESDIR=${MATLAB_ROOT}/X11/app-defaults

