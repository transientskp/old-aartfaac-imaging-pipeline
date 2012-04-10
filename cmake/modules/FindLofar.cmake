# Find the Lofar includes and library -- currently only LofarStMan
#
#  LOFAR_INCLUDES    - Where to find header files.
#  LOFAR_LIBRARIES   - List of libraries when using cppunit.
#  LOFAR_FOUND       - True if cppunit found.

if (NOT LOFAR_FOUND)

    find_path(LOFAR_INCLUDES LofarConstants.h
        HINTS
            ${LOFAR_ROOT}
            $ENV{LOFAR_ROOT}
        PATH_SUFFIXES
            include
            include/Common
    )

    find_library(LOFAR_LIBRARIES lofarstman
        HINTS
            ${LOFAR_ROOT}
            $ENV{LOFAR_ROOT}
        PATH_SUFFIXES
            lib
            lib64
    )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Lofar
        DEFAULT_MSG
        LOFAR_LIBRARIES
        LOFAR_INCLUDES
    )

    MARK_AS_ADVANCED(LOFAR_LIBRARIES LOFAR_INCLUDES)

endif (NOT LOFAR_FOUND)
