#include <unistd.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include "hashpipe.h"

#include "obs_redis.h"
//----------------------------------------------------------
static redisContext * redis_connect(char *hostname, int port) {
//----------------------------------------------------------
    redisContext *c;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            hashpipe_error(__FUNCTION__, c->errstr);
            redisFree(c);   // get rid of the in-error context
            c = NULL;       // indicate error to caller (TODO - does redisFree null the context pointer?)
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
    }

    return(c);

}

//----------------------------------------------------------
static int redis_get(redisContext *c, redisReply ** reply, const char * query) {
//----------------------------------------------------------

    int rv = 0;
    int i;
    char * errstr;


    *reply = (redisReply *)redisCommand(c, query);

    if(*reply == NULL) {
        errstr = c->errstr;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ERROR) {
        errstr = (*reply)->str;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ARRAY) {
        for(i=0; i < (*reply)->elements; i++) {
            if(!(*reply)->element[i]->str) {
                errstr = (char *)"At least one element in the array was empty";
                rv = 1;
                break;
            }
        }
    }
    if(rv) {
        hashpipe_error(__FUNCTION__, "redis query (%s) returned an error : %s", query, errstr);
    }

    return(rv); 
}

//----------------------------------------------------------
int get_obs_info_from_redis(obs_settings_t * obs_settings,  
                            ana_settings_t * ana_settings,
                            telescope_settings_t * telescope_settings,
                            char    *hostname, 
                            int     port) {
//----------------------------------------------------------

    redisContext *c_observatory;
    redisReply *reply;
    int rv = 0;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

	// Local instrument DB
    // TODO make c static?
	// Observatory DB
    c_observatory = redisConnectWithTimeout((char *)hostname, port, timeout);
    if (c_observatory == NULL || c_observatory->err) {
        if (c_observatory) {
            hashpipe_error(__FUNCTION__, c_observatory->errstr);
            redisFree(c_observatory);
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
        exit(1);
    }
    
	// Get observatory data
    char query_string[2048] = {0};
    // get digial settings from redis database
    sprintf(query_string, "hmget %s %s", DIGITAL_KEY, "TimeStamp    \
                                                       SampleFreq   \
                                                       AccLen       \
                                                       AdcCoarseGain\
                                                       FFTShift     \
                                                       Scaling      \
                                                       SpecCoeff    \
                                                       AdcDelay0    \
                                                       AdcDelay1    \
                                                       AdcDelay2    \
                                                       AdcDelay3    \
                                                       AdcDelay4    \
                                                       AdcDelay5    \
                                                       AdcDelay6    \
                                                       AdcDelay7    \
                                                       fpg          \
                                                       data_sel");
    if(!rv) rv = redis_get(c_observatory, &reply, query_string);
	if(!rv)
    {
        obs_settings->TIME      = atof(reply->element[0]->str);	
        obs_settings->SAMPLEFREQ= atoi(reply->element[1]->str);
        obs_settings->ACCLEN    = atoi(reply->element[2]->str);
        obs_settings->ADCCOARSEGAIN = atoi(reply->element[3]->str);
        obs_settings->FFTSHIFT  = atoi(reply->element[4]->str);
        obs_settings->SCALING   = atoi(reply->element[5]->str);
        obs_settings->SPECCOEFF = atoi(reply->element[6]->str);
        obs_settings->ADCDELAY[0] = atoi(reply->element[7]->str);
        obs_settings->ADCDELAY[1] = atoi(reply->element[8]->str);
        obs_settings->ADCDELAY[2] = atoi(reply->element[9]->str);
        obs_settings->ADCDELAY[3] = atoi(reply->element[10]->str);
        obs_settings->ADCDELAY[4] = atoi(reply->element[11]->str);
        obs_settings->ADCDELAY[5] = atoi(reply->element[12]->str);
        obs_settings->ADCDELAY[6] = atoi(reply->element[13]->str);
        obs_settings->ADCDELAY[7] = atoi(reply->element[14]->str);
        memset(obs_settings->FPG,0,FPG_LEN);
        memcpy(obs_settings->FPG,reply->element[15]->str,reply->element[15]->len);
        obs_settings->DATASEL = atoi(reply->element[16]->str);
	}

    // get analog settings
    memset(query_string, 0, 2048);
    sprintf(query_string, "hmget %s %s", ANALOG_KEY, "RF_Lo_Hz");
    if(!rv) rv = redis_get(c_observatory, &reply, query_string);
    if(!rv)
    {
        ana_settings->RF_LO_HZ = atoi(reply->element[0]->str);
    }

    //get telescope settings
    memset(query_string, 0, 2048);
    sprintf(query_string, "hmget %s %s", TELESCOPE_KEY, "Target_RA_Deg  \
                                                         Target_DEC_Deg \
                                                         Pointing_AZ \
                                                         Pointing_EL \
                                                         Pointing_Updated");
    if(!rv) rv = redis_get(c_observatory, &reply, query_string);
    if(!rv)
    {
        memset(telescope_settings->TARGET_RA_DEG,0,COORD_LEN);
        memset(telescope_settings->TARGET_DEC_DEG,0,COORD_LEN);
        memcpy(telescope_settings->TARGET_RA_DEG, reply->element[0]->str, reply->element[0]->len);
        memcpy(telescope_settings->TARGET_DEC_DEG, reply->element[1]->str, reply->element[1]->len);
        telescope_settings->POINTING_AZ_DEG = atof(reply->element[2]->str);
        telescope_settings->POINTING_EL_DEG = atof(reply->element[3]->str);
        telescope_settings->POINTING_UPDATED = atof(reply->element[4]->str);
    }

    if(c_observatory) redisFree(c_observatory);       // TODO do I really want to free each time?

    return rv;         
}

int redis_set(redisContext *c, redisReply ** reply, const char * query){
        int rv = 0;
    int i;
    char * errstr;


    *reply = (redisReply *)redisCommand(c, query);

    if(*reply == NULL) {
        errstr = c->errstr;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ERROR) {
        errstr = (*reply)->str;
        rv = 1;
    } else if((*reply)->type == REDIS_REPLY_ARRAY) {
        for(i=0; i < (*reply)->elements; i++) {
            if(!(*reply)->element[i]->str) {
                errstr = (char *)"At least one element in the array was empty";
                rv = 1;
                break;
            }
        }
    }
    if(rv) {
        hashpipe_error(__FUNCTION__, "redis query (%s) returned an error : %s", query, errstr);
    }

    return(rv); 
}

//----------------------------------------------------------
int set_files_query(char *hostname, int port, char *filename){
//----------------------------------------------------------
    redisContext *c;
    redisContext *c_observatory;
    redisReply *reply;
    int rv = 0;

    char query_string[128];

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

	// Local instrument DB
    // TODO make c static?
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            hashpipe_error(__FUNCTION__, c->errstr);
            redisFree(c);
        } else {
            hashpipe_error(__FUNCTION__, "Connection error: can't allocate redis context");
        }
        exit(1);
    }
    sprintf(query_string, "LPUSH limbo:raw_pspec_files %s", filename);
    rv = redis_set(c, &reply, query_string);
    return rv;
}