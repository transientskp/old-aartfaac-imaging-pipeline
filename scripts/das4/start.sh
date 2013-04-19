#!/bin/bash

set -e

ROOT=/home/${USER}/imaging
PCONFIG=${ROOT}/src/xml/pipelineConfig.xml
SCONFIG=${ROOT}/src/xml/serverConfig.xml
BINARIES=${ROOT}/build

if [ ! -d "$1" ]; then
	echo "ERROR: '$1' should point to a measurement set." >&2
	exit 1
fi

echo "Starting aartfaac-server"
${BINARIES}/aartfaac-server ${SCONFIG} &
sleep 2

echo "Starting aartfaac pipelines"
qsub -V ${ROOT}/scripts/das4/imaging.job "$1" &
sleep 5

echo "Starting aartfaac emulator"
${BINARIES}/aartfaac-emulator "$1" &

exit 0
