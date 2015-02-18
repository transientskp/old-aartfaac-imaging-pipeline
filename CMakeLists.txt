# === Set the project name and specify the required CMAKE version.
project (aartfaac CXX C)
cmake_minimum_required (VERSION 2.6)

# === Compiler options
set (CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -Wno-long-long")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# === Set cmake 3rd library modules path
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# === General options
option (ENABLE_TESTS "Enable/disable test framework" ON)
option (ENABLE_OPENMP "Enable/disable openmp" OFF)
option (ENABLE_PROFILING "Enable/disable profiling" OFF)
option (ENABLE_LOFARSTMAN "Enable/disable lofarstman linking" OFF)
option (ENABLE_NM_TRACE "Enable/disable simplex trace" OFF)
option (ENABLE_SYSLOG "Enable/disable syslog logging" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

if (ENABLE_OPENMP)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  add_definitions(-DENABLE_OPENMP)
  add_definitions(-DEIGEN_DONT_PARALLELIZE)
endif (ENABLE_OPENMP)

if (ENABLE_PROFILING)
  set (CMAKE_BUILD_TYPE "Release")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
  link_libraries(profiler)
endif (ENABLE_PROFILING)

if (ENABLE_LOFARSTMAN)
  find_package (Lofar REQUIRED)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed")
endif (ENABLE_LOFARSTMAN)

if (ENABLE_NM_TRACE)
  add_definitions(-DNM_TRACE)
endif (ENABLE_NM_TRACE)

if (ENABLE_SYSLOG)
  add_definitions(-DENABLE_SYSLOG)
endif (ENABLE_SYSLOG)

# === Project version
execute_process (
  COMMAND git describe --tags --always
  OUTPUT_VARIABLE VERSION
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (cmake/version.h.in ${PROJECT_BINARY_DIR}/version.h)
configure_file (src/Constants.h.in ${PROJECT_BINARY_DIR}/Constants.h)

# === Include sources.
include (cmake/sources.cmake)
include_directories (${PROJECT_BINARY_DIR})

# === Find and include dependencies.
find_package (Eigen3 3.1 REQUIRED)
find_package (FFTW REQUIRED)
find_package (Qt4 COMPONENTS QtCore QtNetwork QtXml QtGui REQUIRED)
find_package (Pelican REQUIRED)
find_package (Casacore REQUIRED)

include_directories (
  ${EIGEN3_INCLUDE_DIR}
  ${FFTW_INCLUDES}
  ${LOFAR_INCLUDES}
  ${PELICAN_INCLUDES}
  ${CASACORE_INCLUDES}
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
)

# === Preprocess qt moc headers
qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# === Create the emulator binary.
set (EMULATOR ${PROJECT_NAME}-emulator)
add_executable (${EMULATOR} ${EMULATOR_SOURCES})

target_link_libraries (${EMULATOR}
  ${QT_LIBRARIES}
  ${LOFAR_LIBRARIES}
  ${PELICAN_LIBRARIES}
  ${CASACORE_LIBRARIES}
  -lpthread
)

# === Create the pipeline binary.
set (PIPELINE ${PROJECT_NAME}-pipeline)
add_executable (${PIPELINE} ${PIPELINE_SOURCES} ${MOC_SOURCES})

target_link_libraries (${PIPELINE}
  ${QT_LIBRARIES}
  ${CASACORE_LIBRARIES}
  -lfftw3f # can't use ${FFTW_LIBRARIES} here...
  -lpthread
  ${PELICAN_LIBRARIES}
)

# === Create the server binary.
set (SERVER ${PROJECT_NAME}-server)
add_executable (${SERVER} ${SERVER_SOURCES} ${MOC_SOURCES})

target_link_libraries (${SERVER}
  ${QT_LIBRARIES}
  ${PELICAN_LIBRARIES}
  -lpthread
)

# === Testing framework
if (ENABLE_TESTS)
  # === Find CppUnit library
  find_package (CppUnit REQUIRED)

  # === Enable the cmake testing framework.
  enable_testing ()

  # === Compile and add tests to cmake framework.
  foreach (TEST ${TESTS})
      add_executable (${TEST} ${${TEST}_SOURCES})
      target_link_libraries (${TEST}
        ${CPPUNIT_LIBRARIES}
        ${QT_LIBRARIES}
        ${PELICAN_LIBRARIES}
        ${CASACORE_LIBRARIES}
        -lfftw3f # can't use ${FFTW_LIBRARIES} here...
        -lpthread
        ${LOFAR_LIBRARIES}
      )
      add_test (${TEST} ${TEST})
  endforeach (TEST)
endif (ENABLE_TESTS)

# === Install project
foreach (SCRIPT ${SCRIPTS})
  configure_file(${SCRIPT}.in ${PROJECT_BINARY_DIR}/${SCRIPT} @ONLY)
  install(PROGRAMS ${CMAKE_BINARY_DIR}/${SCRIPT} DESTINATION bin)
endforeach (SCRIPT)

foreach (DATAFILE ${DATAFILES})
  configure_file(${DATAFILE}.in ${PROJECT_BINARY_DIR}/${DATAFILE} @ONLY)
endforeach (DATAFILE)
install (DIRECTORY ${PROJECT_BINARY_DIR}/data/ DESTINATION share/aartfaac)

install(TARGETS ${SERVER} ${PIPELINE} ${EMULATOR} ${REGRIDDER} RUNTIME DESTINATION bin)

# === Status report
message (STATUS "")
message (STATUS " C++:")
message (STATUS "   C++ Compiler:         ${CMAKE_CXX_COMPILER}")
message (STATUS "   C++ (Release):        ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message (STATUS "   C++ (Debug):          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
message (STATUS "   C++ (RelWithDebInfo): ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message (STATUS "")
message (STATUS " CMakeflags (${CMAKE_PROJECT_NAME} v${VERSION}):")
message (STATUS "   CMAKE_BUILD_TYPE      ${CMAKE_BUILD_TYPE}")
message (STATUS "   ENABLE_TESTS          ${ENABLE_TESTS}")
message (STATUS "   ENABLE_OPENMP         ${ENABLE_OPENMP}")
message (STATUS "   ENABLE_PROFILING      ${ENABLE_PROFILING}")
message (STATUS "   ENABLE_LOFARSTMAN     ${ENABLE_LOFARSTMAN}")
message (STATUS "   ENABLE_NM_TRACE       ${ENABLE_NM_TRACE}")
message (STATUS "")
