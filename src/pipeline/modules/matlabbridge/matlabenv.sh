#!/bin/sh

module load matlab/64/2011a

MATLAB_ROOT=/sara/sw/matlab/64/2011a
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/runtime/glnxa64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/os/glnxa64
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/native_threads
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64/server
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MATLAB_ROOT}/sys/java/jre/glnxa64/jre/lib/amd64
export XAPPLRESDIR=${MATLAB_ROOT}/X11/app-defaults

