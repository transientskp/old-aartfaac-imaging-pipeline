# === Set the project name and specify the required CMAKE version.
project(aartfaac CXX C Fortran)
cmake_minimum_required(VERSION 2.6)

# === Compiler options
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-function -fno-strict-aliasing -Wl,--no-as-needed")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

# === Enable the cmake testing framework.
enable_testing()

# === Project version
execute_process(
  COMMAND git describe --tags
  OUTPUT_VARIABLE GIT_TAG
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_TAG)
  set(VERSION ${GIT_TAG})
else(NOT GIT_TAG)
  execute_process(
    COMMAND git describe --always
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(VERSION ${GIT_SHA1})
endif(GIT_TAG)

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/version.h.in
  ${PROJECT_BINARY_DIR}/version.h
)

# === Set cmake 3rd library modules path
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# === Include sources.
include(cmake/sources.cmake)
include_directories(${PROJECT_BINARY_DIR})

# === Add matlab bridge
add_subdirectory(${PROJECT_SOURCE_DIR}/src/matlabbridge matlabbridge)
include_directories(
  ${MATLABBRIDGE_SOURCE_DIR}/src
)
link_directories(
  ${MATLAB_ROOT}/runtime/glnxa64
  ${MATLABBRIDGE_BINARY_DIR}
)

# === Find Dependencies.
find_package(Pelican REQUIRED)
find_package(Casacore REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtNetwork QtXml REQUIRED)

# === Add directories to the include path (i.e. -I<directory> ...)
include_directories(${PELICAN_INCLUDES})
include_directories(${CASACORE_INCLUDES})
include_directories(${QT_INCLUDE_DIR})
include_directories(${QT_QTCORE_INCLUDE_DIR})

# === Create the emulator binary.
set(EMULATOR ${PROJECT_NAME}-emulator)
add_executable(${EMULATOR} ${EMULATOR_SOURCES})

target_link_libraries(${EMULATOR}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARIES}
  ${CASACORE_LIBRARIES}
  #/usr/local/lib64/liblofarstman.so
)

# === Create the pipeline binary.
set(PIPELINE ${PROJECT_NAME}-pipeline)
add_executable(${PIPELINE} ${PIPELINE_SOURCES})

target_link_libraries(${PIPELINE}
  mbridge
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${PELICAN_LIBRARIES}
)

# === Create the server binary.
set(SERVER ${PROJECT_NAME}-server)
add_executable(${SERVER} ${SERVER_SOURCES})

target_link_libraries(${SERVER}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARIES}
)

# === Doxygen documentation targets.
# include(UseDoxygen)
# set(PDFLATEX_COMPILER TRUE)
# add_doxygen_target(api DoxyfileAPI)
# add_doxygen_target(developer DoxyfileDeveloper)
# add_doxygen_target(user DoxyfileUser)
