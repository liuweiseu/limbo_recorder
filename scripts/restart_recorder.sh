#!/bin/bash

echo "Starting new instances..."

pkill -f "hashpipe -p limbo"

hostname=`hostname -s`
log_timestamp=`date +%Y%m%d_%H%M%S`

if [ $# -eq 0 ]; then
    spectra_eth="enp3s0"
    voltage_eth="enp136s0"
else
    spectra_eth=$1
    voltage_eth=$2
fi

s=`df -h`
if [[ $s == *"/mnt/ramdisk"*  ]]; then
    echo "Voltage data directory exists."
else
    sudo mount -t tmpfs -o rw,size=16G tmpfs /mnt/ramdisk
    echo "ramdisk created."
fi

if [ -d ../data ]; then
    echo "spectra data directory exists."
else
    mkdir ../data
    echo "spectra data directory created."
fi

cd ../data
hashpipe -p limbo.so -I 0 net_thread output_thread  -o PKT_TYPE=0 \
                                                    -o BINDHOST=$spectra_eth \
                                                    1> ${hostname}.spectra.out.${log_timestamp} \
                                                    2> ${hostname}.spectra.err.${log_timestamp} &
echo "Instance 0 started."
echo 'Sepctra data path:' `pwd`

cd /mnt/ramdisk
hashpipe -p limbo.so -I 1 net_thread output_thread  -o PKT_TYPE=2 \
                                                    -o BINDHOST=$voltage_eth \
                                                    1> ${hostname}.voltage.out.${log_timestamp} \
                                                    2> ${hostname}.voltage.err.${log_timestamp} &
echo "Instance 1 started."
echo 'Voltage data path:' `pwd`

