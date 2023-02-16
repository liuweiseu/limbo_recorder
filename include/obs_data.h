#ifndef _OBS_DATA_H
#define _OBS_DATA_H

#include<stdio.h>

#define FPG_LEN         128
#define SW_VER          "0.0.1"
#define VER_STR_LEN     16
#define HDR_SIZE        1024

typedef struct obs_header{
    double TIME;
    char FPG[FPG_LEN];
    char SWVER[VER_STR_LEN];
    unsigned int HDRSIZE;
    unsigned int SAMPLEFREQ;
    unsigned int ACCLEN;
    unsigned int ADCCOARSEGAIN;
    unsigned int FFTSHIFT;
    unsigned int DATASEL;
    unsigned int SCALING;
    unsigned int SPECCOEFF;
    unsigned int ADCDELAY[8];
}obs_header_t;

typedef struct record_status{
    char filename[128];
    unsigned int file_created;
    unsigned int recording;
} record_status_t;

void create_file(char *filename);
int write_header(obs_header_t *obs_header);
int write_data(void *data, int nbytes);
void close_file();

#endif