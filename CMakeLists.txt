# === Set the project name and specify the required CMAKE version.
project(aartfaac CXX C Fortran)
cmake_minimum_required(VERSION 2.6)

# === Components to build
set(BUILD_EMULATOR ON CACHE "Build emulator" BOOL)
set(BUILD_SERVER ON CACHE "Build server" BOOL)
set(BUILD_PIPELINE ON CACHE "Build pipeline" BOOL)

if (NOT BUILD_EMULATOR AND NOT BUILD_SERVER AND NOT BUILD_PIPLEINE)
    message(FATAL_ERROR "Nothing to build")
endif (NOT BUILD_EMULATOR AND NOT BUILD_SERVER AND NOT BUILD_PIPLEINE)

# === Compiler options
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wno-long-long -Wno-unused-function -fno-strict-aliasing -Wl,--no-as-needed")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

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

# === Find Dependencies.
find_package(Qt4 COMPONENTS QtCore QtNetwork QtXml REQUIRED)
include_directories(${QT_INCLUDE_DIR})
include_directories(${QT_QTCORE_INCLUDE_DIR})

find_package(Pelican REQUIRED)
include_directories(${PELICAN_INCLUDES})

find_package(CppUnit REQUIRED)

if (BUILD_EMULATOR OR BUILD_PIPELINE)
    find_package(Casacore REQUIRED)
    include_directories(${CASACORE_INCLUDES})
endif (BUILD_EMULATOR OR BUILD_PIPELINE)

if (BUILD_EMULATOR AND USE_LOFARSTMAN)
    find_package(Lofar REQUIRED)
    include_directories(${LOFAR_INCLUDES})
endif (BUILD_EMULATOR AND USE_LOFARSTMAN)

# === Create the emulator binary.
if (BUILD_EMULATOR)
    set(EMULATOR ${PROJECT_NAME}-emulator)
    add_executable(${EMULATOR} ${EMULATOR_SOURCES})

    target_link_libraries(${EMULATOR}
      ${QT_QTCORE_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${PELICAN_LIBRARIES}
      ${CASACORE_LIBRARIES}
      ${LOFAR_LIBRARIES}
    )
endif (BUILD_EMULATOR)

# === Create the pipeline binary.
if (BUILD_PIPELINE)
    set(PIPELINE ${PROJECT_NAME}-pipeline)
    add_executable(${PIPELINE} ${PIPELINE_SOURCES})

    target_link_libraries(${PIPELINE}
      ${QT_QTCORE_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${QT_QTGUI_LIBRARY}
      ${CASACORE_LIBRARIES}
      ${PELICAN_LIBRARIES}
    )
endif (BUILD_PIPELINE)

# === Create the server binary.
if (BUILD_SERVER)
    set(SERVER ${PROJECT_NAME}-server)
    add_executable(${SERVER} ${SERVER_SOURCES})

    target_link_libraries(${SERVER}
      ${QT_QTCORE_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${PELICAN_LIBRARIES}
    )
endif (BUILD_SERVER)

# === Enable the cmake testing framework.
enable_testing()

# === Compile and add tests to cmake framework.
foreach (TEST ${TESTS})
    add_executable(${TEST} ${${TEST}_SOURCES})
		target_link_libraries(${TEST} 
			${CPPUNIT_LIBRARIES}
      ${QT_QTCORE_LIBRARY}
      ${QT_QTNETWORK_LIBRARY}
      ${QT_QTXML_LIBRARY}
      ${PELICAN_LIBRARIES}
      ${CASACORE_LIBRARIES}
      ${LOFAR_LIBRARIES}
		)
		add_test(${TEST} ${TEST})
endforeach (TEST)

# === Doxygen documentation targets.
# include(UseDoxygen)
# set(PDFLATEX_COMPILER TRUE)
# add_doxygen_target(api DoxyfileAPI)
# add_doxygen_target(developer DoxyfileDeveloper)
# add_doxygen_target(user DoxyfileUser)
