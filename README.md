# LIMBO Recorder
The code is used for recording data for LIMBO project.
## Getting Start
1. compile the code
```
    make
    sudo make install
```
2. Start the data Recorder
```
    cd ./scripts
    ./restart_recorder.sh
```
When you run this script, a "data" directory will be created the project directory.  
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
