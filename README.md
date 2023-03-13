# LIMBO Recorder
The code is used for recording data for LIMBO project.
## Getting Start
1. compile the code
    ```
        mkdir Release
        cd Release
        cmake .. -DCMAKE_BUILE_TYPE=Release
        make
        sudo make install
    ```
2. Set Environment Variables  
You need to set three environment variables:  
(1) LIMBO_DATA_DIR, which defines the spectra data path;  
(2) SPECTRA_ETH, which defines the ethernet port for spectra data;  
(3) VOLTAGE_ETH, which defines the ethernet port for voltage data.  
(***The following values are the default vaules. If you're happy with them, you don't need to set them again.***)
    ```
        export LIMBO_DATA_DIR=/home/obs/data
        export SPECTRA_ETH=enp3s0
        export VOLTAGE_ETH=enp136s0
    ```

2. Start the data Recorder    
    ```
        restart_recorder.sh
    ```
When you run this script, a "data" directory will be created defined by "LIMBO_DATA_DIR", which is used for storing spectra data. Voltage data files will be created in ramdisk(/mnt/ramdisk). A new file will be created every second. The max number of the voltage data files is 16 by default. The oldest file will be deleted automatically, when the file number reaches to the max. The max number of voltage file names is defined in databuf.h
    ```
        #define VOL_FILE_NUM            16
    ```  
***TO-DO***: set port number dynamically. 

3. Enable recording
    ```
        enable_record.sh
    ```
When you run this script, a new data file will be created in the "data" directory, and it stores 4096 spectra data.  

4. Disable recording
    ```
        disbale_record.sh
    ```
This script will stop recording data. Once you run ```enable_record.sh``` again, a new data file will be created.  

5. Stop the data Recorder
    ```
        stop_recorder.sh
    ```
6. mount ramdisk
    ```
        mount_ramdisk.sh 
    ```
Normally, you just need to run this script one time. Then you can run ```df -h``` to make sure the ramdisk is created.
    ```
        ~$ df -h
        Filesystem                         Size  Used Avail Use% Mounted on
        tmpfs                               16G     0   16G   0% /mnt/ramdisk
    ```
7. umount ramdisk
    ```
        umount_ramdisk.sh
    ```
## File Format
We will store two kinds of data: Spectra data and Voltage data.  
### Spectra data file
* file name  
The name of the spectra data file starts with "Spectra_", following the UTC time when the file is generated.  For example "Spectra_20221110042101.dat" means the file was generated at 04:21:01 on 11/10/2022.  
* file header  
We have several versions of file header:  
    1. Ver-0.0.0  
    The size of file header is 1024 bytes for now, which includes the fpg file we used, the UNIX time when we set the registers and the register vaules we used for the experiment. It's [json](https://www.json.org/json-en.html) format. For example:

    ```
        {
            "fpg": "dsa10_frb_2022-11-04_1844.fpg",
            "Time": 1668058079.2594,
        "SampleFreq": 500,
            "AccLen": 127,
    "AdcCoarseGain": 16,
        "FFTShift": 65535,
        "Scaling": 0,
        "DataSel": 1,
        "SpecCoeff": 7,
        "AdcDelay0": 5,
        "AdcDelay1": 5,
        "AdcDelay2": 5,
        "AdcDelay3": 5,
        "AdcDelay4": 5,
        "AdcDelay5": 5,
        "AdcDelay6": 5,
        "AdcDelay7": 5
        }
    ```  
  
    2. Ver-0.0.1  
    The size of file header is defined at the beginning of the file, which is an unsigned int value. **You have to read the first 4 bytes out**, and then you will know the size of file header( **It doesn't include the first 4 bytes**).   
    Some other information are added to the file header, such as RA, DEC, AZ, EL and so on.
    ```
               SWVer: '0.0.1'
                 fpg: 'limbo_500_2022-12-03_1749.fpg'
                Time: 1675998927.58758
          SampleFreq: 500
              AccLen: 127
       AdcCoarseGain: 4
            FFTShift: 2047
             DataSel: 1
             Scaling: 0
           SpecCoeff: 4
           AdcDelay0: 5
           AdcDelay1: 5
           AdcDelay2: 5
           AdcDelay3: 5
           AdcDelay4: 5
           AdcDelay5: 5
           AdcDelay6: 5
           AdcDelay7: 5
            RF_Lo_Hz: 1350000000
       Target_RA_Deg: '271:05:14.85'
       Target_EL_Deg: '-29:31:08.9'
         Pointing_AZ: 0
         Pointing_EL: 90
    Pointing_Updated: 1676526303.31059
    ```
    ***Note***: To recognize the file version, you can read the first byte from the data file. If it's 123("{"), that's the ver-0.0.0 file. 
* data in the file  
Each data frame contains three part:  
1. The time when we received the packets, including second part and micro second part.
    ```
        uint64_t sec
        uint64_t usec
    ```
2. The counter value from FPGA, which counts the frame.  
    ```
        uint64_t cnt
    ```
    ***Note***: Only 56 bits are valid in the cnt value.

3. spectra data
    ```
        uint16_t spectra[2048]
    ```
    2048-channels spectra data are stored, and the data are in 16-bit format.
### Voltage data file
* file name  
The name of the spectra data file starts with "VoltageV2_", following the UTC time when the file is generated.  For example "VoltageV2_20221110042101.dat" means the file was generated at 04:21:01 on 11/10/2022.
* file header  
It's the same as the spectra data file header.   
* data in file  
Each data frame also contains three parts:
1. The time when we received the packets, including second part and micro second part.
    ```
        uint64_t sec
        uint64_t usec
    ```
2. The counter value from FPGA, which counts the frame.  
    ```
        uint64_t cnt
    ```
    ***Note***: Only 56 bits are valid in the cnt value.  

3. Voltage data
    ```
        int8_t voltage[4096]
    ```
    The voltage data contains I/Q ADC inputs(2048 channels x 2). Each channel contains 4-bit real data and 4-bit imaginary data. 
