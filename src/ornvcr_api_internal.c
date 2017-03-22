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
bool*
_ORNVCR_monitor_tracking(void * argument)
{
    struct arg_struct *arg=argument;
    int flag;
    bool rt;
    while(arg->mon->current_index!=0)
    {
    	_ORNVCR_monitor_get_dirtyratio(arg->mon,&flag);
    	sleep((unsigned int)arg->period);
    }
    rt=true;
    pthread_exit(&rt);
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
_ORNVCR_monitor_get_dirtyratio(varMonitor_t *mon, int *flag)
{
	printf("check dirty ratio here\n");
    varProfile_t *temp;
    double max_ratio=0;
    for(temp=mon->hashtableProfile;temp!=NULL;temp=temp->hh.next)
    {
        int rc;
        double ratio;
        
        clock_t tick,tock;
        tick=clock();
        rc=orhash_compute_hash(temp->var_hash);
        tock=clock();
        hash_compute_time+=(double)(tock-tick);
        
        if (rc != ORHASH_SUCCESS)
        {
            fprintf (stderr, "ERROR: orhash_compute_hash() failed (line: %d)\n", __LINE__);
            return false;
        }
        
        tick=clock();
        rc = orhash_get_dirty_ratio (temp->var_hash, &ratio);
        tock=clock();
        compare_time+=(double)(tock-tick);
        
        if (rc != ORHASH_SUCCESS)
        {
            fprintf (stderr, "ERROR: orhash_get_dirty_ratio() failed (line: %d)\n", __LINE__);
            return false;
        }    
        temp->dirty_ratio=ratio;
        if(temp->dirty_ratio>temp->cScheme.dirty_threshold){
            //if any variable's dirty ratio reaches threshold, conduct checkpoint
            *flag=1; 
            
            clock_t tick,tock;
            tick=clock();
            _ORNVCR_checkpoint_routine_all(mon);
            tock=clock();
            checkpoint_time+=(double)(tock-tick);
            
            mon->current_version++;
            return true;
        }
            
        printf("ratio is %f\n",ratio);
    }
    
    return false;
}

/**
* _ORNVCR_checkpoint_routine() dumps the memory of a variable to a storage location 
* It only saves per variable
* @param[in,out]   profile the variable profile of the variable to be checkpointed
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_checkpoint_routine_one(varProfile_t *profile,char * path)
{
    //implement single process checkpoint for now
    FILE* fs=fopen(path,"w");
    if(fs==NULL) printf("unable to open file at %s\n",path);
    int valid=false;
    if(fs!=NULL){
        valid=true;
        size_t n=fwrite(profile->address,profile->type_size,profile->count,fs);
        if(n!=profile->type_size*profile->count){valid = false; }
        if(fclose(fs)!=0) { valid = false; }
    }
    return valid;
    
}

/**
* _ORNVCR_checkpoint_routine_all() dumps the memory of all variables to specified storage locations 
* It saves all variables
* @param[in,out]   mon monitor structure
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_checkpoint_routine_all(varMonitor_t *mon){
    varProfile_t *temp;  
    char path[80];
    for(temp=mon->hashtableProfile;temp!=NULL;temp=temp->hh.next)
    {
        if((temp->dirty_ratio)>0)
        {
            printf("check 1\n");
            _ORNVCR_generate_chkpt_filepath(mon, temp, path);
            printf("check 2\n");
            _ORNVCR_checkpoint_routine_one(temp,path); 
        }
        else
        {
            if(_ORNVCR_find_last_chkptfile(mon,temp)==true)
            {
                _ORNVCR_update_chkptfile_version(path, mon->current_version);
            }
                
            
        }
            
            
    }
}

/**
* _ORNVCR_generate_chkpt_filepath() generate the correct file path for the checkpoint file 
* @param[in,out]   mon monitor structure
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_generate_chkpt_filepath(varMonitor_t *mon, varProfile_t *profile, char *path){
    char tmp_path[80];
    char tmp[20];
    strcpy(tmp_path,profile->chkpt_base_path);
    sprintf(tmp,"/%d_%d.chpt",profile->index,mon->current_version);
    strcat(tmp_path,tmp);
    strcpy(path,tmp_path);
    return true;
    
}

/**
* _ORNVCR_find_last_chkptfile() finds if the last version of checkpoint file exists
* @param[in,out]   mon monitor structure
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_find_last_chkptfile(varMonitor_t *mon, varProfile_t *profile){
    
}


bool _ORNVCR_update_chkptfile_version(char *path, int version){ //rename last checkpoint file to lastest version number
    
}



