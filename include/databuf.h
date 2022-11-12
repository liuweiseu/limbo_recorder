#include <stdint.h>
#include <stdio.h>
#include "hashpipe.h"
#include "hashpipe_databuf.h"

#define DEBUG                   0

#define REDIS_HOST              "localhost"
#define REDIS_PORT              6379

#define CACHE_ALIGNMENT         8
#define N_BLOCKS                64 

#define SPECTRA_SIZE            2048
#define SPECTRAS_PER_BLOCK      (4*1024) 
#define SPECTRA_PKT_SIZE        (SPECTRA_SIZE * sizeof(unsigned short) + 8)
#define SPECTRA_FRAME_SIZE      (SPECTRA_PKT_SIZE + 8*2)
#define SPECTRA_BYTES_PER_BLOCK (SPECTRA_FRAME_SIZE * SPECTRAS_PER_BLOCK)  

#define VOL_SIZE                8192
#define VOL_PER_BLOCK           (2*1024)
#define VOL_PKT_SIZE            (VOL_SIZE * sizeof(char) + 8)
#define VOL_FRAME_SIZE          (VOL_PKT_SIZE + 8*2)
#define VOL_BYTES_PER_BLOCK     (VOL_FRAME_SIZE * VOL_PER_BLOCK)

#define MAX_BYTES_PER_BLOCK     ((VOL_BYTES_PER_BLOCK > SPECTRA_BYTES_PER_BLOCK)?VOL_BYTES_PER_BLOCK:SPECTRA_BYTES_PER_BLOCK)
#define CNT_BITWIDTH            56
/* INPUT BUFFER STRUCTURES
  */
typedef struct block_header {
   uint64_t mcnt;            
} block_header_t;

typedef uint8_t header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(block_header_t)%CACHE_ALIGNMENT)
];

typedef struct spectra_pkt {
    uint64_t cnt;  //id and cnt are combined to a 64-bit data, id-8bit + cnt-56bit
    uint16_t pkt_data[SPECTRA_SIZE];
} spectra_pkt_t;

typedef struct spectra_frame {
    uint64_t tv_sec;
    uint64_t tv_usec;
    spectra_pkt_t packet;
} spectra_frame_t;

typedef struct vol_pkt{
    uint64_t cnt;
    uint8_t pkt_data[VOL_SIZE];
} vol_pkt_t;

typedef struct vol_frame{
    uint64_t tv_sec;
    uint64_t tv_usec;
} vol_frame_t;
typedef struct block {
   block_header_t header;
   header_cache_alignment padding; // Maintain cache alignment
   uint8_t blk_data[MAX_BYTES_PER_BLOCK];
} block_t;

typedef struct databuf {
   hashpipe_databuf_t header;
   header_cache_alignment padding;
   block_t block[N_BLOCKS];
} databuf_t;



/*
 * BUFFER FUNCTIONS
 */
hashpipe_databuf_t *databuf_create(int instance_id, int databuf_id);

static inline databuf_t *databuf_attach(int instance_id, int databuf_id)
{
    return (databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int databuf_detach(databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline void databuf_clear(databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline int databuf_block_status(databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_total_status(databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int databuf_wait_free(databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_busywait_free(databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_wait_filled(databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_busywait_filled(databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_set_free(databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int databuf_set_filled(databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}
