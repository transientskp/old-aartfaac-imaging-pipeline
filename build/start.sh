#!/bin/bash

PIPELINES=8

echo "Starting server"
./aartfaac-server ../src/xml/serverConfig.xml &

for (( i=1; i<=${PIPELINES}; i++ ))
do
	echo "Started ${i}th pipeline"
	./aartfaac-pipeline ../src/xml/pipelineConfig.xml &
done

sleep 1
./aartfaac-emulator $1

