# FindCasacore.cmake
# =============================================================================
#
# CMake script to find the Casacore library associated dependencies.
#
# =============================================================================
# Defines the following variables:
#
#   CASACORE_FOUND             = True if Casacore is found.
#
#   CASACORE_INCLUDES          = Set of include directories needed by Casacore.
#
#   CASACORE_LIBRARY           = The Casacore library.
#   CASACORE_LIBRARIES         = Set of libraries required for linking.
#
# ============================================================================
# Environment and CMake variables effecting this script.
#
#   CASACORE_ROOT = Top level casacore install directory.
#                         This is the root install location of casacore and
#                         if built from source (on linux) will be usually
#                         be either /usr/local or /usr
#
# ============================================================================
# Known Issues
#   - This script is untested with either Mac OS or Windows.
#
# ============================================================================
# Last Update: 27 Feb 2012
# =============================================================================


# Include default error handling macro.
include(FindPackageHandleStandardArgs)

# For convenience, we want two include directories:
#
# 1. ${CASACORE_ROOT}/include
# 2. ${CASACORE_ROOT}/include/casacore/
#
# 1 is convenient for our own code: it lets us #include <casacore/..>
# 2 is *required* for casacore's internal use.
if (NOT CASACORE_INCLUDES)
  find_path(CASACORE_INCLUDES casa/aips.h
    HINTS ${CASACORE_ROOT}
    PATH_SUFFIXES include/casacore
  )
endif(NOT CASACORE_INCLUDES)
if (CASACORE_ROOT)
  list(APPEND CASACORE_INCLUDES ${CASACORE_ROOT}/include)
endif (CASACORE_ROOT)

# Find the Casacore library.
find_library(CASACORE_LIBRARY casacore
    NAMES casa_casa
    PATHS
    ${CASACORE_ROOT}/lib
    $ENV{CASACORE_ROOT}/lib
    /usr/lib
    /usr/local/lib
)
set(CASACORE_LIBRARIES ${CASACORE_LIBRARY})

# Find the Casacore library.
find_library(CASACORE_MS_LIBRARY casams
    NAMES casa_ms
    PATHS
    ${CASACORE_ROOT}/lib
    $ENV{CASACORE_ROOT}/lib
    /usr/lib
    /usr/local/lib
)
list(APPEND CASACORE_LIBRARIES ${CASACORE_MS_LIBRARY})

# Find the Casacore tables library
find_library(CASACORE_TABLES_LIBRARY casacore
    NAMES casa_tables
    PATHS
    ${CASACORE_ROOT}/lib
    $ENV{CASACORE_ROOT}/lib
    /usr/lib
    /usr/local/lib
)
list(APPEND CASACORE_LIBRARIES ${CASACORE_TABLES_LIBRARY})

# Check for errors.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casacore
    "ERROR: Could not find Casacore include directory."
    CASACORE_INCLUDES
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casacore
    "ERROR: Could not find Casacore library."
    CASACORE_LIBRARY
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casatables
    "ERROR: Could not find Casa tables library."
    CASACORE_TABLES_LIBRARY
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casams
    "ERROR: Could not find Casa measureset library."
    CASACORE_MS_LIBRARY
)


# Put variables in the advanced section of the CMake cache.
mark_as_advanced(
    CASACORE_FOUND
    CASACORE_INCLUDES
    CASACORE_LIBRARY
    CASACORE_LIBRARIES
)

