#ifndef _OBS_REDIS_H
#define _OBS_REDIS_H

#define FPG_LEN 128
typedef struct obs_settings{
    double TIME;
    char FPG[FPG_LEN];
    unsigned int SAMPLEFREQ;
    unsigned int ACCLEN;
    unsigned int ADCCOARSEGAIN;
    unsigned int FFTSHIFT;
    unsigned int SCALING;
    unsigned int SPECCOEFF;
    unsigned int ADCDELAY[8];
}obs_settings_t;

int get_obs_info_from_redis(obs_settings_t * obs_settings, char *hostname, int port);

#endif