#!/bin/bash

set -e

ROOT=`pwd`
SCONFIG=${ROOT}/../src/xml/serverConfig.xml
PCONFIG=${ROOT}/../src/xml/pipelineConfig.xml
PIPELINES=2

if [ -z "$2" ]; then
  PIPELINES=2
else
  PIPELINES=$2
fi

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

echo "Starting aartfaac emulator"
${ROOT}/aartfaac-emulator $1 &

exit 0
