/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "ORNVCR.h"
#include "uthash.h"

pthread_t monitor_thread;

bool
ORNVCR_init(varMonitor_t **mon)
{
    varMonitor_t *_m = NULL;

    _m = calloc (1, sizeof (varMonitor_t));
    if (_m == NULL)
    {
        errno = ENOMEM;
        return false;
    }
    _m->hashtableProfile=NULL;
    _m->current_index=0;
    _m->dirtyratio=0;
    _m->latest_checkpoint_time=(struct timeval){.tv_sec=0,.tv_usec=0};
    *mon = _m;

    return true;
}

bool
ORNVCR_exit(varMonitor_t **mon)
{
    if (mon != NULL)
    {
        if (*mon != NULL)
        {
            free (*mon);
            *mon = NULL;
        }
    }

    return true;
}

bool
ORNVCR_register(varMonitor_t *mon, void* var_address, int size, int type, varProfile_t *profile)
{
    
    if(profile==NULL){
        profile=malloc(sizeof(varProfile_t));
        profile->address=var_address;
        profile->index=mon->current_index+1;
        mon->current_index++;
        gettimeofday(&(profile->allocate_time),NULL);
        gettimeofday(&(profile->latest_checkpoint_time),NULL);
        profile->size=size;
        profile->type=type;
        profile->dirty_ratio=0; 
        //placement and cScheme are not assigned yet    
    }
    //add the profile to hash table
    HASH_ADD_INT( mon->hashtableProfile, address, profile);
    //linked list not yet implemented

    //if it is the first variable registered
    //create background thread to check dirty ratio

    if(mon->current_index==1){
        struct arg_struct argument;
        argument.mon=mon;
        argument.period=5;
        printf("create background thread\n");
        pthread_create(&monitor_thread, NULL, (void*)_ORNVCR_monitor_tracking, (void*) &argument);
    }

    /*
    varProfile_t *test;

    // to test if hashtable is working
    
    HASH_FIND_INT( mon->hashtableProfile, &var_address, test);
    if(test->address==var_address)
        return true;
    else
        return false;
    */
    return true;
}

bool
ORNVCR_deregister(varMonitor_t *mon, void* var_address)
{
    varProfile_t *look_var;
    HASH_FIND_INT( mon->hashtableProfile, &var_address, look_var);
    HASH_DEL(mon->hashtableProfile,look_var);
    mon->current_index--;
    if(mon->current_index==0)
    //stop the monitor thread
    {
        bool rc;
        pthread_join(monitor_thread, (void *)&rc);
    }

    return false;
}

bool
ORNVCR_get_profile(varMonitor_t *mon, int var_address, varProfile_t *profile)
{
    return false;
}

bool
ORNVCR_set_profile(varMonitor_t *mon, int var_address, varProfile_t profile)
{
    return false;
}

bool
ORNVCR_need_check(varMonitor_t *mon)
{
    return false;
}

bool
ORNVCR_checkpoint(varMonitor_t *mon)
{
    return false;
}

bool
ORNVCR_check_exist (varMonitor_t *mon)
{
    return false;
}

bool
ORNVCR_restore (varMonitor_t *mon, int var_address, int size)
{
    return false;
}

