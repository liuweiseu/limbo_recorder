#ifndef _OBS_REDIS_H
#define _OBS_REDIS_H

#define FPG_LEN         128
#define COORD_LEN       16

#define DIGITAL_KEY     "OBS_SETTINGS"
#define ANALOG_KEY      "limbo"
#define TELESCOPE_KEY   "limbo"

// this is for digital settings
typedef struct obs_settings{
    double TIME;
    char FPG[FPG_LEN];
    unsigned int SAMPLEFREQ;
    unsigned int ACCLEN;
    unsigned int ADCCOARSEGAIN;
    unsigned int FFTSHIFT;
    unsigned int DATASEL;
    unsigned int SCALING;
    unsigned int SPECCOEFF;
    unsigned int ADCDELAY[8];
}obs_settings_t;

// this is for analog settings
typedef struct ana_settings{
    unsigned int RF_LO_HZ;
}ana_settings_t;

// this is for telescope settings
typedef struct telescope_settings{
    char TARGET_RA_DEG[COORD_LEN];
    char TARGET_DEC_DEG[COORD_LEN];
    double POINTING_AZ_DEG;
    double POINTING_EL_DEG;
    double POINTING_UPDATED;
}telescope_settings_t;

int get_obs_info_from_redis(obs_settings_t * obs_settings, ana_settings_t * ana_settings, telescope_settings_t * telescope_settings,char *hostname, int port);
int set_files_query(char *hostname, int port, char *filename);
#endif