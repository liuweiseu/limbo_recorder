#include<stdio.h>
#include "obs_data.h"

static FILE *fp;

void create_file(char *filename){
    fp = fopen(filename,"w");
}

int write_header(obs_header_t *obs_header){
    // wirte the file size to the beginning of the file.
    // it should be a 4B value.
    fwrite(&(obs_header->HDRSIZE), sizeof(unsigned int), 1, fp);
    // write json header to the file
    fprintf(fp, "{\n");
    fprintf(fp, " \"SWVer\":\"%s\",      \
                  \"fpg\": \"%s\",       \
                  \"Time\": %10f,        \
                  \"SampleFreq\": %4u,   \
                  \"AccLen\": %4u,       \
                  \"AdcCoarseGain\": %4u,\
                  \"FFTShift\": %5u,     \
                  \"DataSel\": %4u,      \
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
                }", obs_header->SWVER,      \
                    obs_header->FPG,        \
                    obs_header->TIME,       \
                    obs_header->SAMPLEFREQ, \
                    obs_header->ACCLEN,     \
                    obs_header->ADCCOARSEGAIN,\
                    obs_header->FFTSHIFT,   \
                    obs_header->DATASEL,    \
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
    // we use the first HDR_SIZE bytes for header
    return fseek(fp, HDR_SIZE + 4, SEEK_SET);
}

int write_data(void *data, int nbytes){
    fwrite((char *)data, 1, nbytes, fp);
}

void close_file(){
    fclose(fp);
}