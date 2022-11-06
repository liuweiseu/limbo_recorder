#include<stdio.h>
#include "include/obs_data.h"

void create_file(char *filename, FILE *fp){
    fp = fopen(filename,"w");
}

int write_header(FILE *fp, obs_header_t *obs_header){
    fprintf(fp, "{\n");
    fprintf(fp, " \"Time\": %10f,        \
                  \"SampleFreq\": %4u,   \
                  \"AccLen\": %4u,       \
                  \"FFTShitf\": %5u,     \
                  \"Scaling\": %4u,      \
                  \"SpecCoeff\": %4u,    \
                  \"AdcDelay0\": %4u,    \
                  \"AdcDelay1\": %4u,    \
                  \"AdcDelay2\": %4u,    \
                  \"AdcDelay3\": %4u,    \
                  \"AdcDelay4\": %4u,    \
                  \"AdcDelay5\": %4u,    \
                  \"AdcDelay6\": %4u,    \
                  \"AdcDelay7\": %4u,    \
                }", obs_header->TIME,       \
                    obs_header->SAMPLEFREQ, \
                    obs_header->ACCLEN,     \
                    obs_header->FFTSHITF,   \
                    obs_header->SCALING,    \
                    obs_header->SPECCOEFF,  \
                    obs_header->ADCDELAY[0],\
                    obs_header->ADCDELAY[1],\
                    obs_header->ADCDELAY[2],\
                    obs_header->ADCDELAY[3],\
                    obs_header->ADCDELAY[4],\
                    obs_header->ADCDELAY[5],\
                    obs_header->ADCDELAY[6],\
                    obs_header->ADCDELAY[7]
                    );
    // we use the first 256 bytes for header
    return fseek(fp, 256, SEEK_SET);
}

int write_data(FILE *fp, void *data, int nbytes){
    fwrite(data, 1, nbytes, fp);
}

void close_file(FILE *fp){
    fclose(fp);
}