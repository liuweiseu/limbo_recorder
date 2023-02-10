/*
 * output_thread.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "hashpipe.h"
#include "databuf.h"
#include "obs_redis.h"
#include "obs_data.h"

/*
create data file name.
para: 
--filename: the created filename
--pkt_type: indicate the packet type.
            If it's "0", it means the packet contains spectra data,
            so the file name will start with "Spectra".
            If it's "1", it means the packet contains voltage data,
            so the file name will start with "Voltage".
*/
static void create_filename(char *filename, unsigned int pkt_type){
    time_t timenow;
    struct tm *tm_p;
    time(&timenow);
    tm_p = gmtime(&timenow);
    int l = 0;
    if(pkt_type == 0)
    {
        strcpy(filename,"Spectra");
        l = strlen("Spectra");
    }
    else if(pkt_type == 1)
    {
        strcpy(filename,"VoltageV1");
        l = strlen("VoltageV1");
    }
    else if(pkt_type == 2)
    {
        strcpy(filename,"VoltageV2");
        l = strlen("VoltageV2");
    }
    sprintf(filename + l,"_%04d%02d%02d%02d%02d%02d.dat", tm_p->tm_year + 1900,  \
                                            tm_p->tm_mon + 1,      \
                                            tm_p->tm_mday,         \
                                            tm_p->tm_hour,         \
                                            tm_p->tm_min,          \
                                            tm_p->tm_sec);
    fprintf(stdout,"New file created: %s\n", filename);    
}

/*
Get SNAP board settings fron redis server, 
and then put the setting into obs_header, which will be written as data file header.
para:
--obs_settings  : store the data from redis server
--obs_header    : store the date, which will be written into file header.
*/
static void get_snap_settings(obs_header_t *obs_header)
{
    obs_settings_t obs_settings;
    obs_settings_t *obs_settings_ptr = &obs_settings;
    get_obs_info_from_redis(obs_settings_ptr,REDIS_HOST, REDIS_PORT);
    obs_header->ACCLEN      = obs_settings_ptr->ACCLEN;
    obs_header->ADCCOARSEGAIN = obs_settings_ptr->ADCCOARSEGAIN;
    memcpy(obs_header->ADCDELAY, obs_settings_ptr->ADCDELAY, 8*sizeof(unsigned int));
    obs_header->FFTSHIFT    = obs_settings_ptr->FFTSHIFT;
    obs_header->SAMPLEFREQ  = obs_settings_ptr->SAMPLEFREQ;
    obs_header->SCALING     = obs_settings_ptr->SCALING;
    obs_header->SPECCOEFF   = obs_settings_ptr->SPECCOEFF;
    obs_header->TIME        = obs_settings_ptr->TIME;
    memcpy(obs_header->FPG, obs_settings_ptr->FPG, FPG_LEN);
    obs_header->DATASEL     = obs_settings_ptr->DATASEL;
}

/*
Scan Voltage files in the directory.
para:
--fname_string: file name should start with this string.
                For example, when pkt_type is 1, it's "VoltageV1",
                             when pkt_type is 2, it's "VoltageV2".  
--file_list   : voltage files in the directory.
                The max number of the voltage files should be 16.
*/
static int scan_voltage_files(char *fname_string, char (*file_list)[128])
{
    struct dirent **entry_list, *entry;
    int count = 0, i = 0, j = 0;
    count = scandir("./",&entry_list, 0, alphasort);
    if(count == 0)
        return 0;
    for(i = 0; i < count; i++)
    {
        entry = entry_list[i];
        if(!strncmp(entry->d_name,fname_string, strlen(fname_string)-1))
        {
            memcpy(file_list[j],entry->d_name, strlen(entry->d_name));
            j++;
        }
    }
}

static int init(hashpipe_thread_args_t *args) {
    // Get info from status buffer if present
    hashpipe_status_t st = args->st;
    printf("\n\n-----------Start Setup of Output Thread--------------\n");

    printf("-----------Finished Setup of Output Thread-----------\n\n");  
    return 0;
}

static void *run(hashpipe_thread_args_t * args)
{
    // Local aliases to shorten access to args fields
    // Our input buffer happens to be a ouput_databuf
    databuf_t *db = (databuf_t *)args->ibuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;
    int c,rv;
    int block_idx = 0;
    uint32_t record_flag = 0;
    uint32_t pkt_type = 0;
    uint32_t file_blocks = 0;
    uint32_t file_count = 0;
    char filename[128]={0};
    char file_list[VOL_FILE_NUM][128] = {0};
    record_status_t recordstatus;
    record_status_t *recordstatus_ptr = &recordstatus;
    memset(recordstatus_ptr->filename,0,128);
    recordstatus_ptr->file_created = 0;
    recordstatus_ptr->recording = 0;
    
    /* scan voltage files first*/
    file_count = scan_voltage_files("Voltage", file_list);
    printf("file_count=%d\n", file_count);
    /* Main loop */
    while (run_threads()) {
        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "OUTBLKIN", block_idx);
        hputs(st.buf, status_key, "waiting");
        hashpipe_status_unlock_safe(&st);
       // get new data
       while ((rv=databuf_wait_filled(db, block_idx))
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for filled databuf");
                pthread_exit(NULL);
                break;
            }
        }
 
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing");
        hgetu4(st.buf, "PKT_TYPE",&pkt_type);
        hashpipe_status_unlock_safe(&st);

        hgetu4(st.buf,"RECORD",&record_flag);
        if(record_flag)
        {
            if(recordstatus_ptr->file_created == 0)
            {
                // before creating a new file, we need to delete an old file,
                // to make sure the max number of files is VOL_FILE_NUM.
                file_count = file_count%VOL_FILE_NUM;
                if(pkt_type != 0)
                    remove(file_list[file_count]);
                memset(file_list[file_count],0,128);
                // if we are going to record data, but the file hasn't been created,
                // let's create the file.
                memset(recordstatus_ptr->filename,0,128);
                create_filename(recordstatus_ptr->filename,pkt_type);
                memcpy(file_list[file_count], recordstatus_ptr->filename, strlen(recordstatus_ptr->filename));
                file_count++;
                create_file(recordstatus_ptr->filename);
                recordstatus_ptr->file_created = 1;
                recordstatus_ptr->recording = 0;
            }
            if(recordstatus_ptr->recording == 0)
            {
                // if the data file is created, we need to write header.
                obs_header_t obs_header;
                get_snap_settings(&obs_header);
                write_header(&obs_header);
                recordstatus_ptr->recording = 1;
            }
            if(recordstatus_ptr->recording == 1)
            {
                if(pkt_type == 0)// spectra data
                {
                    write_data(db->block[block_idx].blk_data,SPECTRAS_PER_BLOCK*SPECTRA_FRAME_SIZE);
                    close_file();
                    recordstatus_ptr->file_created = 0;
                    recordstatus_ptr->recording = 0;
                }
                else if(pkt_type == 1)// voltage data Ver1.0--4 adc inputs
                {
                    write_data(db->block[block_idx].blk_data,VOLV1_PER_BLOCK*VOLV1_FRAME_SIZE);
                    file_blocks++;
                }
                else if(pkt_type == 2)// voltage data Ver2.0--2 adc inputs
                {
                    write_data(db->block[block_idx].blk_data,VOLV2_PER_BLOCK*VOLV2_FRAME_SIZE);
                    file_blocks++;
                }
                if(file_blocks == VOL_FILE_BLOCKS)
                {
                    file_blocks = 0;
                    close_file();
                    recordstatus_ptr->file_created = 0;
                    recordstatus_ptr->recording = 0;
                }
            }  
        }else
        {   
            if(recordstatus_ptr->file_created == 1)
            {
                close_file();
                recordstatus_ptr->file_created = 0;
                recordstatus_ptr->recording = 0;
            }
        }
        databuf_set_free(db,block_idx);
        block_idx = (block_idx + 1) % db->header.n_block;
    }
    return NULL;
}

static hashpipe_thread_desc_t output_thread = {
    name: "output_thread",
    skey: "OUTSTAT",
    init: init, 
    run:  run,
    ibuf_desc: {databuf_create},
    obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&output_thread);
}

