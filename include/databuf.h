#include <stdint.h>
#include <stdio.h>
#include "hashpipe.h"
#include "hashpipe_databuf.h"

#define DEBUG                   0

#define REDIS_HOST              "localhost"
#define REDIS_PORT              6379

#define CACHE_ALIGNMENT         8
#define N_INPUT_BLOCKS          8 
#define N_OUTPUT_BLOCKS         8

#define SPECTRA_SIZE            2048
#define SPECTRAS_PER_BLOCK      4*1024
#define PKT_SIZE                (SPECTRA_SIZE * sizeof(unsigned short) + 8)
#define FRAME_SIZE              (PKT_SIZE + 8*2)   
#define CNT_BITWIDTH            56

/* INPUT BUFFER STRUCTURES
  */
typedef struct input_block_header {
   uint64_t mcnt;            
} input_block_header_t;

typedef uint8_t input_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(input_block_header_t)%CACHE_ALIGNMENT)
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

typedef struct input_block {
   input_block_header_t header;
   input_header_cache_alignment padding; // Maintain cache alignment
   frame_t spectra[SPECTRAS_PER_BLOCK];
} input_block_t;

typedef struct input_databuf {
   hashpipe_databuf_t header;
   input_header_cache_alignment padding;
   input_block_t block[N_INPUT_BLOCKS];
} input_databuf_t;


/*
  * OUTPUT BUFFER STRUCTURES
  */
typedef struct output_block_header {
    uint64_t mcnt;
} output_block_header_t;

typedef uint8_t output_header_cache_alignment[
   CACHE_ALIGNMENT - (sizeof(output_block_header_t)%CACHE_ALIGNMENT)
];

typedef struct output_block {
    output_block_header_t header;
    output_header_cache_alignment padding; // Maintain cache alignment
    frame_t spectra[SPECTRAS_PER_BLOCK] ;
} output_block_t;

typedef struct output_databuf {
   hashpipe_databuf_t header;
   output_header_cache_alignment padding;
   //hashpipe_databuf_cache_alignment padding; // Maintain cache alignment
   output_block_t block[N_OUTPUT_BLOCKS];
} output_databuf_t;

/*
 * INPUT BUFFER FUNCTIONS
 */
hashpipe_databuf_t *input_databuf_create(int instance_id, int databuf_id);

static inline input_databuf_t *input_databuf_attach(int instance_id, int databuf_id)
{
    return (input_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int input_databuf_detach(input_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline void input_databuf_clear(input_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline int input_databuf_block_status(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_total_status(input_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int input_databuf_wait_free(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_busywait_free(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_wait_filled(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_busywait_filled(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_set_free(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int input_databuf_set_filled(input_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}

/*
 * OUTPUT BUFFER FUNCTIONS
 */

hashpipe_databuf_t *output_databuf_create(int instance_id, int databuf_id);

static inline void output_databuf_clear(output_databuf_t *d)
{
    hashpipe_databuf_clear((hashpipe_databuf_t *)d);
}

static inline output_databuf_t *output_databuf_attach(int instance_id, int databuf_id)
{
    return (output_databuf_t *)hashpipe_databuf_attach(instance_id, databuf_id);
}

static inline int output_databuf_detach(output_databuf_t *d)
{
    return hashpipe_databuf_detach((hashpipe_databuf_t *)d);
}

static inline int output_databuf_block_status(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_block_status((hashpipe_databuf_t *)d, block_id);
}

static inline int output_databuf_total_status(output_databuf_t *d)
{
    return hashpipe_databuf_total_status((hashpipe_databuf_t *)d);
}

static inline int output_databuf_wait_free(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_free((hashpipe_databuf_t *)d, block_id);
}

static inline int output_databuf_busywait_free(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_free((hashpipe_databuf_t *)d, block_id);
}
static inline int output_databuf_wait_filled(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_wait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int output_databuf_busywait_filled(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_busywait_filled((hashpipe_databuf_t *)d, block_id);
}

static inline int output_databuf_set_free(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_free((hashpipe_databuf_t *)d, block_id);
}

static inline int output_databuf_set_filled(output_databuf_t *d, int block_id)
{
    return hashpipe_databuf_set_filled((hashpipe_databuf_t *)d, block_id);
}


