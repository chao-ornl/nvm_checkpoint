/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "ORNVCR.h"

/**
*the following code may be not neccessary because we are using explicit mon in external APIs.
**
**
**
bool
_ORNVCR_monitor_init(varMonitor_t **mon)
{

    return false;
}

bool
_ORNVCR_monitor_register(varMonitor_t **mon, void * var_buff, varProfile *profile)
{

    return false;
}
**
**
**
*/


/**
* _ORNVCR_monitor_tracking() calls _ORNVCR_monitor_get_dirtyratio periodically to get the dirty ratio
* @param[in,out]   mon      Opaque handle representing the internal state of the
*                           infrastructure. It is set to NULL upon success to
*                           prevent any further usage.
*                  period   The time interval of calling _ORNVCR_monitor_get_dirtyratio
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool
_ORNVCR_monitor_tracking(void * argument)
{
    struct arg_struct *arg=argument;
    while(arg->mon->current_index!=0)
    {
    	_ORNVCR_monitor_get_dirtyratio(arg->mon);
    	sleep((unsigned int)arg->period);
    }
    return true;
}

/**
* _ORNVCR_monitor_get_dirtyratio() use liborhash’s function to calculate the new hash and the dirty ratio. 
* It iterate through all varProfiles from monitor->headProfile.
* @param[in,out]   mon Opaque handle representing the internal state of the
*                      infrastructure. It is set to NULL upon success to
*                      prevent any further usage.
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool
_ORNVCR_monitor_get_dirtyratio(varMonitor_t *mon)
{
	printf("check dirty ratio here\n");
    varProfile_t *temp;
    double max_ratio=0;
    for(temp=mon->hashtableProfile;temp!=NULL;temp=temp->hh.next)
    {
        int rc;
        double ratio;
        rc=orhash_compute_hash(temp->var_hash);
        if (rc != ORHASH_SUCCESS)
        {
            fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
            return false;
        }
        rc = orhash_get_dirty_ratio (temp->var_hash, &ratio);
        if (rc != ORHASH_SUCCESS)
        {
            fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
            return false;
        }    
        temp->dirty_ratio=ratio;
        printf("ratio is %f\n",ratio);
    }
    
    return false;
}



