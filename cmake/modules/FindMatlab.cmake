# Find the native Matlab includes and library
#
#  MCC_EXECUTABLE      - The mcc compiler.
#  MATLAB_INCLUDES     - Required headers.
#  MATLAB_LIBRARIES    - Required libraries.
#  MATLAB_FOUND        - True if Matlab found.

if (NOT MATLAB_FOUND)

    find_program(MCC_EXECUTABLE mcc
        HINTS
            ${MATLAB_ROOT}
            $ENV{MATLAB_ROOT}
        PATH_SUFFIXES
            bin
    )

    find_path(MATLAB_INCLUDES mclmcrrt.h
        HINTS
            ${MATLAB_ROOT}
            $ENV{MATLAB_ROOT}
        PATH_SUFFIXES
            extern/include
    )

    find_library(MATLAB_LIBRARIES libmwmclmcrrt.so
        HINTS
            ${MATLAB_ROOT}
            $ENV{MATLAB_ROOT}
        PATH_SUFFIXES
            runtime/glnxa64
    )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Matlab
        DEFAULT_MSG
        MCC_EXECUTABLE
        MATLAB_INCLUDES
        MATLAB_LIBRARIES
    )

    MARK_AS_ADVANCED(MCC_EXECUTABLE MATLAB_INCLUDES MATLAB_LIBRARIES)

endif (NOT MATLAB_FOUND)
