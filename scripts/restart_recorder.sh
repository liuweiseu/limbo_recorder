#!/bin/bash

echo "Starting new instances..."

pkill -f "hashpipe -p limbo"

hostname=`hostname -s`
log_timestamp=`date +%Y%m%d_%H%M%S`

if [ -z ${LIMBO_DATA_DIR} ];then 
    limbo_data_dir="/home/obs/data"
else
    limbo_data_dir=${LIMBO_DATA_DIR}
fi

if [ -z ${SPECTRA_ETH} ];then 
    spectra_eth="enp3s0"
else
    spectra_eth=${SPECTRA_ETH}
fi

if [ -z ${VOLTAGE_ETH} ];then 
    voltage_eth="enp136s0"
else
    voltage_eth=${VOLTAGE_ETH}
fi

echo "Spectra Data Ethernet Port: ${spectra_eth}"
echo "Voltage Data Ethernet Port: ${voltage_eth}"

s=`df -h`
if [[ $s == *"/mnt/ramdisk"*  ]]; then
    echo "Voltage data directory exists."
else
    sudo mount -t tmpfs -o rw,size=16G tmpfs /mnt/ramdisk
    echo "ramdisk created."
fi

if [ -d ${limbo_data_dir} ]; then
    echo "spectra data directory exists."
else
    mkdir ${limbo_data_dir}
    echo "spectra data directory created."
fi

cd ${limbo_data_dir}
hashpipe -p liblimbo.so -I 0 net_thread output_thread  -o PKT_TYPE=0 \
                                                    -o BINDHOST=$spectra_eth \
                                                    1> ${hostname}.spectra.out.${log_timestamp} \
                                                    2> ${hostname}.spectra.err.${log_timestamp} &
echo "Instance 0 started."
echo 'Sepctra data path:' `pwd`

cd /mnt/ramdisk
hashpipe -p liblimbo.so -I 1 net_thread output_thread  -o PKT_TYPE=2 \
                                                    -o BINDHOST=$voltage_eth \
                                                    1> ${hostname}.voltage.out.${log_timestamp} \
                                                    2> ${hostname}.voltage.err.${log_timestamp} &
echo "Instance 1 started."
echo 'Voltage data path:' `pwd`

