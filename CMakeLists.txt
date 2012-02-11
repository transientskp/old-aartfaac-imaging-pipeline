# === Set the project name and specify the required CMAKE version.
project(aartfaac CXX C Fortran)
cmake_minimum_required(VERSION 2.8)

# === Compiler options
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

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

# === Enable the cmake testing framework.
enable_testing()

# === Find Dependencies.
find_package(Pelican REQUIRED)
find_package(CppUnit REQUIRED)
find_package(Casacore REQUIRED)

# === Add directories to the include path (i.e. -I<directory> ...)
include_directories(${PELICAN_INCLUDES})

# === Create the emulator binary.
set(EMULATOR ${PROJECT_NAME}-emulator)
add_executable(${EMULATOR} ${EMULATOR_SOURCES})

target_link_libraries(${EMULATOR}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARY}
  ${CASACORE_LIBRARIES}
)

# === Create the server binary.
set(SERVER ${PROJECT_NAME}-server)
add_executable(${SERVER} ${SERVER_SOURCES})

target_link_libraries(${SERVER}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARY}
)

# === Create the pipeline binary.
set(PIPELINE ${PROJECT_NAME}-pipeline)
add_executable(${PIPELINE} ${PIPELINE_SOURCES})

target_link_libraries(${PIPELINE}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARY}
)

# === Doxygen documentation targets.
# include(UseDoxygen)
# set(PDFLATEX_COMPILER TRUE)
# add_doxygen_target(api DoxyfileAPI)
# add_doxygen_target(developer DoxyfileDeveloper)
# add_doxygen_target(user DoxyfileUser)
