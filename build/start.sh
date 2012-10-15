#!/bin/bash

set -e

ROOT=`pwd`
SCONFIG=${ROOT}/../src/xml/serverConfig.xml
PCONFIG=${ROOT}/../src/xml/pipelineConfig.xml
PIPELINES=2

if [ ! -d "$1" ]; then
	echo "ERROR: <arg1> should point to a measurement set." >&2
	exit 1
fi

echo "Starting aartfaac-server"
${ROOT}/aartfaac-server ${SCONFIG} &
sleep 2

echo "Starting ${PIPELINES} aartfaac pipelines"
for (( i=0; i<${PIPELINES}; i++ )); do
	${ROOT}/aartfaac-pipeline ${PCONFIG} &
done;
sleep 5

echo "Starting aartfaac service emulator"
${ROOT}/aartfaac-service-emulator $1 &
sleep 5

echo "Starting aartfaac stream emulator"
${ROOT}/aartfaac-stream-emulator $1 &

exit 0
