#ifndef _OBS_REDIS_H
#define _OBS_REDIS_H

typedef struct obs_settings{
    double TIME;
    unsigned int SAMPLEFREQ;
    unsigned int ACCLEN;
    unsigned int FFTSHITF;
    unsigned int SCALING;
    unsigned int SPECCOEFF;
    unsigned int ADCDELAY[8];
}obs_settings_t;

int get_obs_info_from_redis(obs_settings_t * obs_settings, char *hostname, int port);

#endif