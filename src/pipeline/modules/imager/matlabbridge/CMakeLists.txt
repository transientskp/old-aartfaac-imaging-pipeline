cmake_minimum_required(VERSION 2.8)
project(IMAGER)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif(NOT CMAKE_BUILD_TYPE)

set(MATLAB_ROOT /usr/local/MATLAB/R2011b CACHE PATH "Location of matlab installation")

link_directories(
	${MATLAB_ROOT}/runtime/glnxa64
	${IMAGER_BINARY_DIR}
)

include_directories(
	${MATLAB_ROOT}/extern/include
	${IMAGER_BINARY_DIR}
)

set(SOURCE_FILES
	src/cpp/matlabbridge.cpp
	src/cpp/matlabbridge.h
	${IMAGER_BINARY_DIR}/libmatlabbridge.h
)

set(MM_FILES
	${IMAGER_SOURCE_DIR}/src/matlab/fft_imager.m
	)

add_library(imager STATIC ${SOURCE_FILES})

if(CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
	set(MCC_FLAGS -g)
else()
	set(MCC_FLAGS)
endif()

if(CMAKE_VERBOSE_MAKEFILE MATCHES ON)
	set(MCC_FLAGS ${MCC_FLAGS} -v)
endif()

add_custom_command(
	OUTPUT libmatlabbridge.cpp libmatlabbridge.h libmatlabbridge.exports libmatlabbridge.so mccExcludedFiles.log readme.txt
	DEPENDS ${MM_FILES}
	COMMAND ${MATLAB_ROOT}/bin/mcc -B cpplib:libmatlabbridge
	${MCC_FLAGS} ${MM_FILES}
)

target_link_libraries(imager matlabbridge mwmclmcrrt)
