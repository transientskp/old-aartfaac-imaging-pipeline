# === Set the project name and specify the required CMAKE version.
project (aartfaac CXX C Fortran)
cmake_minimum_required (VERSION 2.6)

# === Compiler options
set (CMAKE_C_FLAGS "-Wall -Wextra")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set (CMAKE_CXX_FLAGS "-Wall -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DQT_NO_DEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# === General options
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

option (ENABLE_TESTS "Enable/disable test framework" ON)

# === Project version
execute_process (
  COMMAND git describe --tags
  OUTPUT_VARIABLE GIT_TAG
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (GIT_TAG)
  set (VERSION ${GIT_TAG})
else (NOT GIT_TAG)
  execute_process (
    COMMAND git describe --always
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set (VERSION ${GIT_SHA1})
endif (GIT_TAG)

configure_file (
  ${PROJECT_SOURCE_DIR}/cmake/version.h.in
  ${PROJECT_BINARY_DIR}/version.h
)

# === Set cmake 3rd library modules path
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# === Include sources.
include (cmake/sources.cmake)
include_directories (${PROJECT_BINARY_DIR})

# === Find and include dependencies.
find_package (Qt4 COMPONENTS QtCore QtNetwork QtXml REQUIRED)
include_directories (${QT_INCLUDE_DIR})
include_directories (${QT_QTCORE_INCLUDE_DIR})

find_package (Pelican REQUIRED)
include_directories (${PELICAN_INCLUDES})

find_package (Casacore REQUIRED)
include_directories (${CASACORE_INCLUDES})

find_package (Lofar REQUIRED)
include_directories (${LOFAR_INCLUDES})

# === Create the emulator binary.
set (EMULATOR ${PROJECT_NAME}-emulator)
add_executable (${EMULATOR} ${EMULATOR_SOURCES})

target_link_libraries (${EMULATOR}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARIES}
  ${CASACORE_LIBRARIES}
  ${LOFAR_LIBRARIES}
)

# === Create the pipeline binary.
set (PIPELINE ${PROJECT_NAME}-pipeline)
add_executable (${PIPELINE} ${PIPELINE_SOURCES})

target_link_libraries (${PIPELINE}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${CASACORE_LIBRARIES}
  ${PELICAN_LIBRARIES}
)

# === Create the server binary.
set (SERVER ${PROJECT_NAME}-server)
add_executable (${SERVER} ${SERVER_SOURCES})

target_link_libraries (${SERVER}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${PELICAN_LIBRARIES}
)

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
        ${QT_QTCORE_LIBRARY}
        ${QT_QTNETWORK_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${PELICAN_LIBRARIES}
        ${CASACORE_LIBRARIES}
        ${LOFAR_LIBRARIES}
      )
      add_test (${TEST} ${TEST})
  endforeach (TEST)
endif (ENABLE_TESTS)

# === Doxygen documentation targets.
# include (UseDoxygen)
# set (PDFLATEX_COMPILER TRUE)
# add_doxygen_target (api DoxyfileAPI)
# add_doxygen_target (developer DoxyfileDeveloper)
# add_doxygen_target (user DoxyfileUser)

# === Status report
message (STATUS "")
message (STATUS " C/C++:")
message (STATUS "   C++ Compiler:               ${CMAKE_CXX_COMPILER}")
message (STATUS "   C++ flags (Release):        ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message (STATUS "   C++ flags (Debug):          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
message (STATUS "   C++ flags (RelWithDebInfo): ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message (STATUS "   C Compiler:                 ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1}")
message (STATUS "   C flags (Release):          ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
message (STATUS "   C flags (Debug):            ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
message (STATUS "   C flags (RelWithDebInfo):   ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
message (STATUS "")
message (STATUS " CMakeflags (${CMAKE_PROJECT_NAME}):")
message (STATUS "   CMAKE_BUILD_TYPE            ${CMAKE_BUILD_TYPE}")
message (STATUS "   ENABLE_TESTS                ${ENABLE_TESTS}")
message (STATUS "")
