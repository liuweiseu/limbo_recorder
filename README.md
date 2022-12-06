# LIMBO Recorder
The code is used for recording data for LIMBO project.
## Getting Start
1. compile the code
```
    make
    sudo make install
```
2. Start the data Recorder    
* record spectra data
```
    cd ./scripts
    ./restart_recorder.sh
```
When you run this script, a "data" directory will be created the project directory.  
* record voltage data
```
    ./restart_recorder.sh voltagev2
```
When you run this script, the data files will be created in ramdisk(/mnt/ramdisk). A new file will be created every second. The max number of the voltage data files is 16 by default. The oldest file will be deleted automatically, when the file number reaches to the max.  
The max number of voltage file names is defined in databuf.h
```
    #define VOL_FILE_NUM            16
```
***TO-DO***: set the ethernet port and port number dynamically.   
3. Enable recording
```
    ./enable_record.sh
```
When you run this script, a data file will be created in the "data" directory.  

4. Disable recording
```
    ./disbale_record.sh
```
This script will stop recording data. Once you run ```./enable_record.sh``` again, a new data file will be created.  

5. Stop the data Recorder
```
    ./stop_recorder.sh
```
6. mount ramdisk
```
    ./mount_ramdisk.sh 
```
Normally, you just need to run this script one time. Then you can run ```df -h``` to make sure the ramdisk is created.
```
    ~$ df -h
    Filesystem                         Size  Used Avail Use% Mounted on
    tmpfs                               16G     0   16G   0% /mnt/ramdisk
```
7. umount ramdisk
```
    ./umount_ramdisk.sh
```
## File Format
We will store two kind of data: Spectra data and Voltage data.  
### Spectra data file
* file name  
The name of the spectra data file starts with "Spectra_", following the UTC time when the file is generated.  For example "Spectra_20221110042101.dat" means the file was generated at 04:21:01 on 11/10/2022.  
* file header  
The size of file header is 1024 bytes for now, which includes the fpg file we used, the UNIX time when we set the registers and the register vaules we used for the experiment. It's [json](https://www.json.org/json-en.html) format. For example:
```
    {
           "fpg": "dsa10_frb_2022-11-04_1844.fpg",
          "Time": 1668058079.2594,
    "SampleFreq": 500,
        "AccLen": 127,
      "FFTShitf": 65535,
       "Scaling": 0,
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
***Notice***: Only 56 bits are valid in the cnt value.

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
***Notice***: Only 56 bits are valid in the cnt value.
3. Voltage data
```
    int8_t voltage[4096]
```
The voltage data contains I/Q ADC inputs(2048 channels x 2). Each channel contains 4-bit real data and 4-bit imaginary data. 
