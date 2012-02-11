# FindCasacore.cmake
# =============================================================================
#
# CMake script to find the Casacore library associated dependencies.
#
# Dependencies for Casacore searched for
# - fits
#
# =============================================================================
# Defines the following variables:
#
#   CASACORE_FOUND             = True if Casacore is found.
#
#   CASACORE_INCLUDE_DIR       = Top level Casacore include directory.
#   CASACORE_INCLUDES          = Set of include directories needed by Casacore.
#
#   CASACORE_LIBRARY           = The Casacore library.
#   CASACORE_LIBRARIES         = Set of libraries required for linking.
#
# ============================================================================
# Environment and CMake variables effecting this script.
#
#   CASACORE_INSTALL_DIR = Top level casacore install directory.
#                         This is the root install location of casacore and
#                         if built from source (on linux) will be usually
#                         be either /usr/local or /usr
#
# ============================================================================
# Known Issues
#   - This script is untested with either Mac OS or Windows.
#
# ============================================================================
# Last Update: 11 Feb 2012
# =============================================================================


# Include default error handling macro.
include(FindPackageHandleStandardArgs)


# Find the top level Casacore include directory.
find_path(CASACORE_INCLUDE_DIR casa tables
    PATHS
    PATH_SUFFIXES casacore
    ${CASACORE_INSTALL_DIR}/include
    $ENV{CASACORE_INSTALL_DIR}/include
    /usr/include
    /usr/local/include
)
set(CASACORE_INCLUDES ${CASACORE_INCLUDE_DIR})


# Find the Casacore library.
find_library(CASACORE_LIBRARY casacore
    NAMES casa_casa
    PATHS
    ${CASACORE_INSTALL_DIR}/lib
    $ENV{CASACORE_INSTALL_DIR}/lib
    /usr/lib
    /usr/local/lib
)
set(CASACORE_LIBRARIES ${CASACORE_LIBRARY})

# Find the Casacore tables library
find_library(CASACORE_TABLES_LIBRARY casacore
    NAMES casa_tables
    PATHS
    ${CASACORE_INSTALL_DIR}/lib
    $ENV{CASACORE_INSTALL_DIR}/lib
    /usr/lib
    /usr/local/lib
)
list(APPEND CASACORE_LIBRARIES ${CASACORE_TABLES_LIBRARY})

# Check for errors.
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casacore
    "ERROR: Could not find Casacore include directory."
    CASACORE_INCLUDE_DIR
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casacore
    "ERROR: Could not find Casacore library."
    CASACORE_LIBRARY
)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Casatables
    "ERROR: Could not find Casatables library."
    CASACORE_TABLES_LIBRARY
)


# Find dependencies.
#find_package(ccfits)
#list(APPEND CASACORE_INCLUDES
#)
include_directories(${CASACORE_INCLUDES})


# Put variables in the advanced section of the CMake cache.
mark_as_advanced(
    CASACORE_FOUND
    CASACORE_INCLUDE_DIR
    CASACORE_INCLUDES
    CASACORE_LIBRARY
    CASACORE_LIBRARIES
)

