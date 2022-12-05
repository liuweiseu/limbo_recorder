#!/bin/bash

echo "Starting a new instance..."

pkill -f "hashpipe -p limbo"

hostname=`hostname -s`
log_timestamp=`date +%Y%m%d_%H%M%S`

if [ $# -eq 0 ]; then
    pkt_type=0
    if [ -d ../data ]; then
        cd ../data
    else
        mkdir ../data
        cd ../data
    fi
elif [ $1 = 'voltagev1' ]; then
    pkt_type=1
    cd /mnt/ramdisk
elif [ $1 = 'voltagev2' ]; then
    pkt_type=2
    cd /mnt/ramdisk
else 
    echo 'Invalid parameter.'
    exit
fi

hashpipe -p limbo.so -I 0 net_thread output_thread 1> ${hostname}.out.${log_timestamp} \
                                                   2> ${hostname}.err.${log_timestamp} &

hashpipe_check_status -I 0 -k PKT_TYPE -i $pkt_type

echo "Instance 0 started."
echo 'data path:' `pwd`