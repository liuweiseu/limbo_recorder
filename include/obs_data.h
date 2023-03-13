#ifndef _OBS_DATA_H
#define _OBS_DATA_H

#include<stdio.h>

#define FPG_LEN         128
#define SW_VER          "0.0.1"
#define VER_STR_LEN     6
#define HDR_SIZE        2048
#define COORD_LEN       16
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
    unsigned int RF_LO_HZ;
    char TARGET_RA_DEG[COORD_LEN];
    char TARGET_DEC_DEG[COORD_LEN];
    double POINTING_AZ_DEG;
    double POINTING_EL_DEG;
    double POINTING_UPDATED;
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