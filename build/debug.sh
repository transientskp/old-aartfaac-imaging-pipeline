#!/bin/bash

MAX_SLEEP_TIME=8
DBG_FILE=dbgoutput.txt

if [ ! -d "$1" ]; then
	echo "ERROR: <arg1> should point to a measurement set." >&2
	exit 1
fi

rm -vf $DBG_FILE

iterations=0

control_c()
{
  echo "Caught signal interrupt, ran $iterations iterations"
  sh stop.sh
  exit 0
}

trap control_c SIGINT

while true
do
  iterations=$((iterations + 1))
  gdb --command gdbfile --batch --args aartfaac-server ../src/xml/serverConfig.xml &>> $DBG_FILE &
  sleep 1
	./aartfaac-pipeline ../src/xml/pipelineConfig.xml $1 &
  sleep 2
  ./aartfaac-emulator $1 &

  number=$RANDOM
  let "number %= $MAX_SLEEP_TIME"
  echo $number
  sleep $number
  sh stop.sh
done

