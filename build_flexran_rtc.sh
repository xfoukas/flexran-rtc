#!/bin/bash

source flexran_rtc_env

COMMAND="cmake"
ARGS=""

unset $NO_REST
unset $NO_REALTIME

rm "$FLEXRAN_RTC_HOME/CMakeCache.txt"

while getopts rn option
do
    case "${option}"
    in
	r) NO_REALTIME=1;;
	n) NO_REST=1;;
    esac
done

if [ -z $NO_REALTIME ]; then
    echo "Compiling with real-time support"
else
    echo "Compiling without real-time support"
    ARGS="$ARGS -DLOWLATENCY=OFF"
fi

if [ -z $NO_REST ]; then
    echo "Compiling with REST API enabled"
else
    echo "Compiling without REST API"
    ARGS="$ARGS -DREST_NORTHBOUND=OFF"
fi

cmake $ARGS . && make
