#!/bin/bash

set -e

usage()
{
cat << EOF
usage: `basename $0` options

This script starts an imaging pipeline.

OPTIONS:
  -h  Show this message
  -m  MeasurementSet directory
  -p  Number of pipelines (default 1)
EOF
}

ROOT=`pwd`
SCONFIG=$ROOT/../src/xml/serverConfig.xml
PCONFIG=$ROOT/../src/xml/pipelineConfig.xml
PIPELINES=1
MS=

while getopts "hm:p:" OPTION
do
  case $OPTION in
    h)
      usage
      exit 0
      ;;
    m)
      MS=$OPTARG
      ;;
    p)
      PIPELINES=$OPTARG
      ;;
    ?)
      usage
      exit 1
      ;;
  esac
done

if [[ -z $MS ]] || [[ ! -d $MS ]]
then
  usage
  exit 1
fi

echo "Starting aartfaac-server"
CPUPROFILE=/tmp/server.prof $ROOT/aartfaac-server $SCONFIG &
sleep 1

echo "Starting $PIPELINES aartfaac pipelines"
for (( i=0; i<$PIPELINES; i++ ))
do
  CPUPROFILE=/tmp/pipeline.prof $ROOT/aartfaac-pipeline $PCONFIG $MS &
done;
sleep 2

echo "Starting aartfaac emulator"
CPUPROFILE=/tmp/emulator.prof $ROOT/aartfaac-emulator $MS &

exit 0
