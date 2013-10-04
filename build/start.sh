#!/bin/bash

set -e

ROOT=$(pwd)
ECONFIG=$ROOT/../src/xml/emulatorConfig.xml
SCONFIG=$ROOT/../src/xml/serverConfig.xml
PCONFIG=$ROOT/../src/xml/pipelineConfig.xml
PIPELINES=1
OPTIONSET=O1

usage()
{
cat << EOF
usage: `basename $0` options

This script starts an imaging pipeline.

OPTIONS:
  -h  Show this message
  -p  Number of pipelines (default $PIPELINES)
  -o  Option set in emulator (default $OPTIONSET)
EOF
}

while getopts "ho:p:" OPTION
do
  case $OPTION in
    h)
      usage
      exit 0
      ;;
    p)
      PIPELINES=$OPTARG
      ;;
    o)
      OPTIONSET=$OPTARG
      ;;
    ?)
      usage
      exit 1
      ;;
  esac
done

echo "Starting aartfaac-server"
CPUPROFILE=/tmp/server.prof $ROOT/aartfaac-server $SCONFIG &
sleep 1

echo "Starting $PIPELINES aartfaac pipelines"
for (( i=0; i<$PIPELINES; i++ ))
do
  CPUPROFILE=/tmp/pipeline-$i.prof $ROOT/aartfaac-pipeline $PCONFIG &
done;
sleep 2

echo "Starting aartfaac emulator"
CPUPROFILE=/tmp/emulator.prof $ROOT/aartfaac-emulator $ECONFIG $OPTIONSET &

exit 0
