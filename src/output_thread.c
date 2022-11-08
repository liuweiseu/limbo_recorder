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

#include "hashpipe.h"
#include "include/databuf.h"
#include "include/obs_redis.h"
#include "include/obs_data.h"

static void create_filename(char *filename, unsigned int pkt_type){
    time_t timenow;
    struct tm *tm_p;
    tm_p = localtime(&timenow);
    sprintf(filename,"%4d%2d%2d%2d%2d%2d");
#ifdef DEBUG
    //fprintf(stdout,"%s\n",filename);
    printf("%s\n", filename);    
#endif
}

static void *run(hashpipe_thread_args_t * args)
{
    // Local aliases to shorten access to args fields
    // Our input buffer happens to be a ouput_databuf
    output_databuf_t *db = (output_databuf_t *)args->ibuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;
    int c,rv;
    int block_idx = 0;
    uint32_t record_flag = 0;
    char filename[128]={0};
    /* Main loop */
    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "OUTBLKIN", block_idx);
        hputs(st.buf, status_key, "waiting");
        hashpipe_status_unlock_safe(&st);
       // get new data
       while ((rv=output_databuf_wait_filled(db, block_idx))
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
        hashpipe_status_unlock_safe(&st);

        hgetu4(st.buf,"RECORD",&record_flag);
        if(record_flag){
            for(int i=0; i<sizeof(filename);i++)filename[i] = 0;
            create_file(filename);
        }

        output_databuf_set_free(db,block_idx);
        block_idx = (block_idx + 1) % db->header.n_block;
    }
    return NULL;
}

static hashpipe_thread_desc_t output_thread = {
    name: "output_thread",
    skey: "OUTSTAT",
    init: NULL, 
    run:  run,
    ibuf_desc: {input_databuf_create},
    obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&output_thread);
}

