/*
 * fake_net_thread.c
 *
 * Routine to write fake data into shared memory blocks.  This allows the
 * processing pipelines to be tested without the network portion of SERENDIP6.
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
#include "databuf.h"

#define PKTSOCK_BYTES_PER_FRAME (8192)
#define PKTSOCK_FRAMES_PER_BLOCK (512)
#define PKTSOCK_NBLOCKS (512)
#define PKTSOCK_NFRAMES (PKTSOCK_FRAMES_PER_BLOCK * PKTSOCK_NBLOCKS)

// Initialization function for Hashpipe.
// This function is called once when the thread is created
// args: Arugments passed in by hashpipe framework.
static int init(hashpipe_thread_args_t * args){
    printf("\n\n-----------Start Setup of Net Thread--------------\n");
    // define default network params
    char bindhost[128];
    int bindport = 4000;
    hashpipe_status_t st = args->st;
    strcpy(bindhost, "0.0.0.0");

    // Lock shared buffer to properly get and set values.
    hashpipe_status_lock_safe(&st);

    // Get info from status buffer if present
    hgets(st.buf, "BINDHOST", 128, bindhost);
    hgeti4(st.buf, "BINDPORT", &bindport);

    // Store bind host/port info and other info in status buffer
    hputs(st.buf, "BINDHOST", bindhost);
	hputi4(st.buf, "BINDPORT", bindport);
    hputi8(st.buf, "NPACKETS", 0);
    hputu4(st.buf, "RECORD", 0);

    // Unlock shared buffer once complete.
    hashpipe_status_unlock_safe(&st);

    // Set up pktsocket
    struct hashpipe_pktsock *p_ps = (struct hashpipe_pktsock *)
    malloc(sizeof(struct hashpipe_pktsock));

    if(!p_ps) {
        perror(__FUNCTION__);
        return -1;
    }

    // Make frame_size be a divisor of block size so that frames will be
	// contiguous in mapped mempory.  block_size must also be a multiple of
	// page_size.  Easiest way is to oversize the frames to be 16384 bytes, which
	// is bigger than we need, but keeps things easy.
    // 
	p_ps->frame_size = PKTSOCK_BYTES_PER_FRAME;

	// total number of frames
	p_ps->nframes = PKTSOCK_NFRAMES;

	// number of blocks
	p_ps->nblocks = PKTSOCK_NBLOCKS;

    // Opening Pktsocket to receive data.
    int rv = hashpipe_pktsock_open(p_ps, bindhost, PACKET_RX_RING);
	if (rv!=HASHPIPE_OK) {
        hashpipe_error("net_thread", "Error opening pktsock.");
        pthread_exit(NULL);
	}

    // Store packet socket pointer in args
	args->user_data = p_ps;

    // Initialize the the starting values of the input buffer.
    input_databuf_t *db  = (input_databuf_t *)args->obuf;
    printf("-----------Finished Setup of Input Thread------------\n\n");
	// Success!
    return 0;
}

static void *run(hashpipe_thread_args_t * args)
{
    input_databuf_t *db  = (input_databuf_t *)args->obuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    /* Main loop */
    int i, rv,input;
    uint64_t mcnt = 0;
    int block_idx = 0;
 
    uint8_t* pkt_data;
    struct timeval nowTime;             // Current NTP UTC time
    uint32_t id;                        //snap id
    uint32_t pkt_type = 0;              // pkt type, 0: spectra; 1: voltage(500MSps); 2: voltage(1000MSps)
    uint32_t pktsock_pkts = 0;          // Stats counter for socket packet
    uint32_t pktsock_drops = 0;         // Stats counter for dropped socket packet
    uint32_t pktsock_loss = 0;          // Stats counter for packets loss
    int64_t pktsock_cnt = 0;            // the bit witdth of the cnt value is 56bits 
    int64_t pktsock_pre_cnt = 0;
    uint8_t first_pkt = 0;
    uint32_t npackets = 0;              // number of received packets
    int32_t bindport = 0;

    hashpipe_status_lock_safe(&st);
	hgeti4(st.buf, "BINDPORT", &bindport);
	hputs(st.buf, status_key, "running");
	hashpipe_status_unlock_safe(&st);

    // Get pktsock from args
	struct hashpipe_pktsock * p_ps = (struct hashpipe_pktsock*)args->user_data;
	//pthread_cleanup_push(free, p_ps);
	//pthread_cleanup_push((void (*)(void *))hashpipe_pktsock_close, p_ps);

    // Drop all packets to date
	unsigned char *p_frame;
	while(p_frame=hashpipe_pktsock_recv_frame_nonblock(p_ps)) {
		hashpipe_pktsock_release_frame(p_frame);
	}

    while (run_threads()) {
        // Update the info of the buffer
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "waiting");
        hashpipe_status_unlock_safe(&st);

        // Wait for data
        /* Wait for new block to be free, then clear it
         * if necessary and fill its header with new values.
         */
        while ((rv=input_databuf_wait_free(db, block_idx)) != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }
        
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "receiving");
        hashpipe_status_unlock_safe(&st);

        for(int i = 0; i < SPECTRAS_PER_BLOCK; i++)
        {
             // Recv all of the UDP packets from PKTSOCK
            do {
                p_frame = hashpipe_pktsock_recv_udp_frame_nonblock(p_ps, bindport);
            } while (!p_frame && run_threads());

            // Time stamp the packets and pass it into the shared buffer
            rv = gettimeofday(&nowTime, NULL);
            if (rv == 0){
                db->block[block_idx].spectra->tv_sec = nowTime.tv_sec;
                db->block[block_idx].spectra->tv_usec = nowTime.tv_usec;
            } else {
                fprintf(stderr, "gettimeofday() failed, errno = %d\n", errno);
                db->block[block_idx].spectra->tv_sec = 0;
                db->block[block_idx].spectra->tv_usec = 0;
            }

            pkt_data = (uint8_t *) PKT_UDP_DATA(p_frame);
            // the first 8 bytes are the pkt header
            // cnt(7 bytes) + snap_id(1 byte)
            id = *(pkt_data +7);
            *(pkt_data +7) = 0;
            // we need to adjust the byte order for cnt
            uint8_t tmp;
            tmp = *(pkt_data+6);
            *(pkt_data+6) = *(pkt_data+0);
            *(pkt_data+0) = tmp;
            tmp = *(pkt_data+5);
            *(pkt_data+5) = *(pkt_data+1);
            *(pkt_data+1) = tmp;
            tmp = *(pkt_data+4);
            *(pkt_data+4) = *(pkt_data+2);
            *(pkt_data+2) = tmp;
            // the first 16 bytes are used for sec and usec
            memcpy(db->block[block_idx].spectra+i*PKT_SIZE+16, pkt_data, PKT_SIZE);
            // get the cnt value from the packet
            pktsock_cnt = *((uint64_t *)pkt_data);
            if(first_pkt == 0){
                // if this is the first packet, we don't have a previous packet
                pktsock_pre_cnt = pktsock_cnt - 1;
                first_pkt = 1;
            }
            pktsock_loss += pktsock_pre_cnt + 1 - pktsock_cnt;

            pktsock_pre_cnt = (pktsock_cnt==pow(2,CNT_BITWIDTH)-1)?-1:pktsock_cnt;
            npackets++;

            // Release the hashpipe frame back to the kernel to gather data
            hashpipe_pktsock_release_frame(p_frame);
        }

        // Get stats from packet socket
		hashpipe_pktsock_stats(p_ps, &pktsock_pkts, &pktsock_drops);

        hashpipe_status_lock_safe(&st);
        hputu4(st.buf, "SNAP_ID", id);
        hputu4(st.buf, "PKTTYPE", pkt_type);
        hputi4(st.buf, "NETBKOUT", block_idx);
        hputi8(st.buf, "NETMCNT",mcnt);    
		hputu8(st.buf, "NPACKETS", npackets);	
		hputu4(st.buf, "NETRECV",  pktsock_pkts);
		hputu4(st.buf, "NETDROPS", pktsock_drops);
        hputi8(st.buf, "PKTLOSS",pktsock_loss);
		hashpipe_status_unlock_safe(&st);

        // Mark block as full
        if(input_databuf_set_filled(db, block_idx) != HASHPIPE_OK){
            hashpipe_error(__FUNCTION__, "error waiting for databuf filled call");
            pthread_exit(NULL);
        }
        // Setup for next block
        block_idx = (block_idx + 1) % db->header.n_block;
        db->block[block_idx].header.mcnt = mcnt;
        mcnt++;

        /* Will exit if thread has been cancelled */
        pthread_testcancel();
    }

    // Thread success!
    return THREAD_OK;
}

static hashpipe_thread_desc_t net_thread = {
    name: "net_thread",
    skey: "NETSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {NULL},
    obuf_desc: {input_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&net_thread);
}
