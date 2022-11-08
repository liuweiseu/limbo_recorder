#ifndef _OBS_DATA_H
#define _OBS_DATA_H

#include<stdio.h>

// Now, the header only contains obs settings, but we may put more parameters here.
typedef struct obs_header{
    double TIME;
    unsigned int SAMPLEFREQ;
    unsigned int ACCLEN;
    unsigned int FFTSHITF;
    unsigned int SCALING;
    unsigned int SPECCOEFF;
    unsigned int ADCDELAY[8];
}obs_header_t;

void create_file(char *filename, FILE *fp);
int write_header(FILE *fp, obs_header_t *obs_header);
int write_data(FILE *fp, void *data, int nbytes);
void close_file(FILE *fp);

#endif