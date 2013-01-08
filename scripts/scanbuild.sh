#!/bin/bash

set +e

cd build
sh clean.sh
cmake .. -DCASACORE_ROOT=/usr/local/casacore -DLOFAR_ROOT=/usr/local/LofIm -DCMAKE_C_COMPILER=/usr/share/clang/scan-build/ccc-analyzer -DCMAKE_CXX_COMPILER=/usr/share/clang/scan-build/c++-analyzer -DENABLE_TESTS=ON

if [ -z "${WORKSPACE}" ]; then
  WORKSPACE=`pwd`
fi

# temp directory to store the scan-build report
SCAN_BUILD_TMPDIR=$( mktemp -d /tmp/scan-build.XXXXXX )
 
# directory to use for archiving the scan-build report
SCAN_BUILD_ARCHIVE="${WORKSPACE}/scan-build-archive"
 
# generate the scan-build report
scan-build -k -o ${SCAN_BUILD_TMPDIR} make -j 5
 
# get the directory name of the report created by scan-build
SCAN_BUILD_REPORT=$( find ${SCAN_BUILD_TMPDIR} -maxdepth 1 -not -empty -not -name `basename ${SCAN_BUILD_TMPDIR}` )
rc=$?

cd ..

if [ ! -d "${SCAN_BUILD_ARCHIVE}" ]; then
  echo ">>> Creating scan-build archive directory"
  install -d -o ${USER} -g ${USER} -m 0755 "${SCAN_BUILD_ARCHIVE}"
else
  echo ">>> Removing any previous scan-build reports from ${SCAN_BUILD_ARCHIVE}"
  rm -f ${SCAN_BUILD_ARCHIVE}/*
fi

if [ -z "${SCAN_BUILD_REPORT}" ]; then
  echo ">>> No new bugs identified."
  echo ">>> No scan-build report has been generated"
  rm -f ${SCAN_BUILD_ARCHIVE}/*
  touch ${SCAN_BUILD_ARCHIVE}/index.html
else
  echo ">>> New scan-build report generated in ${SCAN_BUILD_REPORT}"

  echo ">>> Archiving scan-build report to ${SCAN_BUILD_ARCHIVE}"
  mv ${SCAN_BUILD_REPORT}/* ${SCAN_BUILD_ARCHIVE}/

  echo ">>> Removing any temporary files and directories"
  rm -rf "${SCAN_BUILD_TMPDIR}"
fi
 
exit ${rc}
