/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#ifndef ORNVCR_INCLUDE_H
#define ORNVCR_INCLUDE_H

#include <stdbool.h>
#include "ORNVCR_types.h"

/**
 * The ORNVCR_init() function (1) allocates necessary variables with proper
 * data structures defined in the ORCR API library, such as, user program
 * variable table; (2) reads a configuration file with global/default
 * checkpoint/restart policy; and (3) creates “checkpoint thread” if we
 * apply the “proactive” checkpoint shema; etc.
 *
 * @param[in,out]  mon  Opaque handle representing the internal state of the
 *                      infrastructure
 * @return TRUE upon success.
 * @return FALSE upon error, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; ENOMEM: There is
 * insufficient space for the data structures, or the new thread if it is
 * created.
 */
bool
ORNVCR_init(varMonitor_t **mon);

/**
 * The ORNVCR_exit() function releases the resource used for the checkpoint
 * system and exits.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure. It is set to NULL upon success to
 *                      prevent any further usage.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_exit(varMonitor_t **mon);

/**
 * The ORNVCR_register() function registers the program variable allocated to
 * the ORNVCR management system. The function is called exactly after a variable
 * is allocated. The var_address parameter specifies the variable for which to
 * register. The profile parameter specifies the profile (including the
 * checkpoint schema) for the variable.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; ENOMEM: There is
 * insufficient space for the variable table, etc.
 */
bool
ORNVCR_register(varMonitor_t *mon, void* var_address, int size, int type, varProfile_t *profile);

/**
 * The ORNVCR_deregister() function releases the profile (including the
 * checkpoint scheme) of a given variable. When a variable is released by call
 * to free(), this function could be used to release the profile. The var_address
 * parameter specifies the variable.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; EINVAL: The options
 * argument is not valid.
 */
bool
ORNVCR_deregister(varMonitor_t *mon, void* var_address);

/**
 * The ORNVCR_get_profile() function returns the profile (including the
 * checkpoint scheme) of a given variable. The var_address parameter
 * specifies the variable. The profile parameter saves the profile returned.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_get_profile(varMonitor_t *mon, int var_address, varProfile_t *profile);

/**
 * The ORNVCR_set_profile() function sets the profile (including the checkpoint
 * scheme) of a given variable. When a variable is reallocated by call to
 * realloc(), this function could be used to update the profile. The
 * var_address parameter specifies the variable. The profile parameter
 * specifies the profile (including the checkpoint schema) for the variable.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_set_profile(varMonitor_t *mon, int var_address, varProfile_t profile);

/**
 * The ORNVCR_need_check() function checks whether a checkpoint is necessary. The
 * checkpoint system will first check the dirty ratio and variable profile and
 * then makes decision on whether to checkpoint based on the global status of all
 * the variables.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_need_check(varMonitor_t *mon);

/**
 * The ORNVCR_checkpoint() function executes the checkpoint based on the
 * variable profile table.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_checkpoint(varMonitor_t *mon);

/**
 *  The ORNVCR_check_exist () function checks if checkpoint file exists at
 *  predefined locations.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_check_exist (varMonitor_t *mon);

/**
 * The ORNVCR_restore () function (1) loads the hash table from checkpoint file,
 * (2) use the index to locate the entry for the variable to restore in the
 * hash table, (3) locate the checkpoint file and offset for the variable to
 * restore, (4) copy size data from checkpoint file to var_address. To restart,
 * a program allocates and register variables as usual. However, variables will
 * have new addresses such the hash table will has different keys for each
 * variable. When ORNVCR_restore() is called, the new variable is recovered
 * from checkpoint file, using the index parameter. For iterations, the
 * timestep should be checkpointed and restored so that it can restart at the
 * correct timestep.
 *
 * @param[in,out]   mon Opaque handle representing the internal state of the
 *                      infrastructure.
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_restore (varMonitor_t *mon, int var_address, int size);


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
bool *
_ORNVCR_monitor_tracking(void * argument);

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
_ORNVCR_monitor_get_dirtyratio(varMonitor_t *mon, int *flag);



/**
* _ORNVCR_checkpoint_routine_one() dumps the memory of a variable to a storage location 
* It only saves per variable
* @param[in,out]   profile the variable profile of the variable to be checkpointed
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_checkpoint_routine_one(varProfile_t *profile,char * path);

/**
* _ORNVCR_checkpoint_routine_all() dumps the memory of all variables to specified storage locations 
* It saves all variables
* @param[in,out]   mon monitor structure
* @return TRUE upon success.
* @return FALSE upon errors, and the global variable errno is set to indicate
* the error; EINVAL: The options argument is not valid.
*/
bool _ORNVCR_checkpoint_routine_all(varMonitor_t *mon);

bool _ORNVCR_generate_chkpt_filepath(varMonitor_t *mon, varProfile_t *profile, char *path);

bool _ORNVCR_find_last_chkptfile(varMonitor_t *mon, varProfile_t *profile);

bool _ORNVCR_update_chkptfile_version(char *path, int version);

#endif /* ORNVCR_INCLUDE_H */

/* vim: set tabstop=4:shiftwidth=4:expandtab:textwidth=78:filetype=C */
