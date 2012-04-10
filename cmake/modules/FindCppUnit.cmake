# Find the native CppUnit includes and library
#
#  CPPUNIT_INCLUDES    - Where to find header files.
#  CPPUNIT_LIBRARIES   - List of libraries when using cppunit.
#  CPPUNIT_FOUND       - True if cppunit found.

if (NOT CPPUNIT_FOUND)

    find_path(CPPUNIT_INCLUDES TestSuite.h
        HINTS
            ${CPPUNIT_ROOT}
            $ENV{CPPUNIT_ROOT}
        PATH_SUFFIXES
            include
            include/cppunit
    )

    find_library(CPPUNIT_LIBRARIES cppunit
        HINTS
            ${CPPUNIT_ROOT}
            $ENV{CPPUNIT_ROOT}
        PATH_SUFFIXES
            lib
            lib64
    )

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        CppUnit
        DEFAULT_MSG
        CPPUNIT_LIBRARIES
        CPPUNIT_INCLUDES
    )

    MARK_AS_ADVANCED(CPPUNIT_LIBRARIES CPPUNIT_INCLUDES)

endif (NOT CPPUNIT_FOUND)
