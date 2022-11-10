#include<stdio.h>
#include "obs_data.h"

static FILE *fp;

void create_file(char *filename){
    fp = fopen(filename,"w");
}

int write_header(obs_header_t *obs_header){
    fprintf(fp, "{\n");
    fprintf(fp, " \"fpg\": \"%s\",         \
                  \"Time\": %10f,        \
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
                  \"AdcDelay7\": %4u     \
                }", obs_header->FPG,        \
                    obs_header->TIME,       \
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
    // we use the first 1024 bytes for header
    return fseek(fp, 1024, SEEK_SET);
}

int write_data(void *data, int nbytes){
    fwrite((char *)data, 1, nbytes, fp);
}

void close_file(){
    fclose(fp);
}