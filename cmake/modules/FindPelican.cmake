# FindPelican.cmake
# =============================================================================
#
# CMake script to find the Pelican library.
#
# =============================================================================
# Defines the following variables:
#
#   PELICAN_FOUND             = True if Pelican is found.
#   PELICAN_INCLUDES          = Set of include directories needed by Pelican.
#   PELICAN_LIBRARIES         = Set of libraries required for linking.
#
# ============================================================================
# Environment and CMake variables effecting this script.
#
#   PELICAN_ROOT = Top level pelican install directory.
#                         This is the root install location of pelican and
#                         if built from source (on linux) will be usually
#                         be either /usr/local or /usr
#
# ============================================================================
# Known Issues
#   - This script is untested with either Mac OS or Windows.
#
# ============================================================================

if (NOT PELICAN_FOUND)
    # Include default error handling macro.
    include(FindPackageHandleStandardArgs)

    # Find the top level Pelican include directory.
    find_path(PELICAN_INCLUDES pelican
        HINTS
            ${PELICAN_ROOT}
            $ENV{PELICAN_ROOT}
        PATH_SUFFIXES
            include
    )

    # Find the Pelican library.
    find_library(PELICAN_LIBRARIES pelican
        HINTS
            ${PELICAN_ROOT}
            $ENV{PELICAN_ROOT}
        PATH_SUFFIXES
            lib
            lib64
    )

    # Check for errors.
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Pelican
        "ERROR: Could not find Pelican include directory."
        PELICAN_INCLUDES
    )
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Pelican
        "ERROR: Could not find Pelican library."
        PELICAN_LIBRARIES
    )

    # Put variables in the advanced section of the CMake cache.
    mark_as_advanced(
        PELICAN_FOUND
        PELICAN_INCLUDES
        PELICAN_LIBRARIES
    )

endif (NOT PELICAN_FOUND)
