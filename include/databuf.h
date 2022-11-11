#include <stdint.h>
#include <stdio.h>
#include "hashpipe.h"
#include "hashpipe_databuf.h"

#define DEBUG                   0

#define REDIS_HOST              "localhost"
#define REDIS_PORT              6379

#define CACHE_ALIGNMENT         8
#define N_BLOCKS          8 
#define N_OUTPUT_BLOCKS         8

#define SPECTRA_SIZE            2048
#define SPECTRAS_PER_BLOCK      4*1024
#define PKT_SIZE                (SPECTRA_SIZE * sizeof(unsigned short) + 8)
#define FRAME_SIZE              (PKT_SIZE + 8*2)   
#define CNT_BITWIDTH            56

/* INPUT BUFFER STRUCTURES
  */
typedef struct block_header {
   uint64_t mcnt;            
} block_header_t;

typedef uint8_t header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(block_header_t)%CACHE_ALIGNMENT)
];

typedef struct pkt {
    uint64_t cnt;  //id and cnt are combined to a 64-bit data, id-8bit + cnt-56bit
    uint16_t spectra[SPECTRA_SIZE];
} pkt_t;

typedef struct frame {
    uint64_t tv_sec;
    uint64_t tv_usec;
    pkt_t pkt_data;
}frame_t;

typedef struct block {
   block_header_t header;
   header_cache_alignment padding; // Maintain cache alignment
   frame_t spectra[SPECTRAS_PER_BLOCK];
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
