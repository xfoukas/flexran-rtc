#!/bin/bash

source flexran_rtc_env

if [ -f $FLEXRAN_RTC_EXEC/rt_controller ]; then

    SUDO='sudo -E'
    SUPERUSER=$SUDO
    
    BIN_FILE="$FLEXRAN_RTC_EXEC/rt_controller"

    while getopts up:n: option
    do
	case "${option}"
	in
	    p) AGENT_PORT=${OPTARG};;
	    n) REST_PORT=${OPTARG};;
	    u) SUPERUSER=""
	esac
    done

    if [ -z $AGENT_PORT ]; then
	echo "Using default agent port (2210)"
    else
	exe_arguments="$exe_arguments -p $AGENT_PORT"
    fi

    if [ -z $REST_PORT ]; then
	echo "Using default REST port (9999)"
    else
	exe_arguments="$exe_arguments -n $REST_PORT"
    fi
    
    exec $SUPERUSER $BIN_FILE $exe_arguments
    
else
    echo "Binary rt_controller not found"
    echo "You need to build FlexRAN controller first"
fi
