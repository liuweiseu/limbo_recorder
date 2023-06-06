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
                  \"Pol0EqCoeff\": %4u,  \
                  \"Pol1EqCoeff\": %4u,  \
                  \"SpecCoeff\": %4u,    \
                  \"AdcDelay0\": %4u,    \
                  \"AdcDelay1\": %4u,    \
                  \"AdcDelay2\": %4u,    \
                  \"AdcDelay3\": %4u,    \
                  \"AdcDelay4\": %4u,    \
                  \"AdcDelay5\": %4u,    \
                  \"AdcDelay6\": %4u,    \
                  \"AdcDelay7\": %4u,     \
                  \"RF_Lo_Hz\" : %10u,    \
                  \"Source\":\"%s\",  \
                  \"Target_RA_Deg\":\"%s\",  \
                  \"Target_DEC_Deg\":\"%s\",  \
                  \"Pointing_AZ\": %2.2f,    \
                  \"Pointing_EL\": %2.2f,    \
                  \"Pointing_Updated\": %10f \
                }", obs_header->SWVER,      \
                    obs_header->FPG,        \
                    obs_header->TIME,       \
                    obs_header->SAMPLEFREQ, \
                    obs_header->ACCLEN,     \
                    obs_header->ADCCOARSEGAIN,\
                    obs_header->FFTSHIFT,   \
                    obs_header->DATASEL,    \
                    obs_header->SCALING,    \
                    obs_header->POL0EQCOEFF,\
                    obs_header->POL1EQCOEFF,\
                    obs_header->SPECCOEFF,  \
                    obs_header->ADCDELAY[0],\
                    obs_header->ADCDELAY[1],\
                    obs_header->ADCDELAY[2],\
                    obs_header->ADCDELAY[3],\
                    obs_header->ADCDELAY[4],\
                    obs_header->ADCDELAY[5],\
                    obs_header->ADCDELAY[6],\
                    obs_header->ADCDELAY[7],\
                    obs_header->RF_LO_HZ,   \
                    obs_header->SOURCE,\
                    obs_header->TARGET_RA_DEG,\
                    obs_header->TARGET_DEC_DEG,\
                    obs_header->POINTING_AZ_DEG,\
                    obs_header->POINTING_EL_DEG,\
                    obs_header->POINTING_UPDATED
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