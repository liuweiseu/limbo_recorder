#!/bin/bash

echo "Starting a new instance..."

pkill -f "hashpipe -p limbo"

hostname=`hostname -s`
log_timestamp=`date +%Y%m%d_%H%M%S`

if [ -d ../data ]; then
    cd ../data
else
    mkdir ../data
    cd ../data
fi

hashpipe -p limbo.so -I 0 net_thread output_thread 1> ${hostname}.out.${log_timestamp} \
                                                   2> ${hostname}.err.${log_timestamp} &

echo "Instance 0 started."
