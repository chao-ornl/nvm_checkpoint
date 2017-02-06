/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#ifndef ORNVCR_INCLUDE_H
#define ORNVCR_INCLUDE_H

#include "ORNVCR_types.h"

/**
 * The ORNVCR_init() function (1) allocates necessary variables with proper
 * data structures defined in the ORCR API library, such as, user program
 * variable table; (2) reads a configuration file with global/default
 * checkpoint/restart policy; and (3) creates “checkpoint thread” if we
 * apply the “proactive” checkpoint shema; etc.
 *
 * @return TRUE upon success.
 * @return FALSE upon error, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; ENOMEM: There is
 * insufficient space for the data structures, or the new thread if it is
 * created.
 */
bool
ORNVCR_init(void);

/**
 * The ORNVCR_exit() function releases the resource used for the checkpoint
 * system and exits.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_exit(void);

/**
 * The ORNVCR_register() function registers the program variable allocated to
 * the ORNVCR management system. The function is called exactly after a variable
 * is allocated. The var_address parameter specifies the variable for which to
 * register. The profile parameter specifies the profile (including the
 * checkpoint schema) for the variable.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; ENOMEM: There is
 * insufficient space for the variable table, etc.
 */
bool
ORNVCR_register(void* var_address, varProfile_t profile);

/**
 * The ORNVCR_deregister() function releases the profile (including the
 * checkpoint scheme) of a given variable. When a variable is released by call
 * to free(), this function could be used to release the profile. The var_address
 * parameter specifies the variable.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid; EINVAL: The options
 * argument is not valid.
 */
bool
ORNVCR_deregister(void* var_address);

/**
 * The ORNVCR_get_profile() function returns the profile (including the
 * checkpoint scheme) of a given variable. The var_address parameter
 * specifies the variable. The profile parameter saves the profile returned.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_get_profile(int var_address, varProfile_t *profile);

/**
 * The ORNVCR_set_profile() function sets the profile (including the checkpoint
 * scheme) of a given variable. When a variable is reallocated by call to
 * realloc(), this function could be used to update the profile. The
 * var_address parameter specifies the variable. The profile parameter
 * specifies the profile (including the checkpoint schema) for the variable.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_set_profile(int var_address, varProfile_t profile);

/**
 * The ORNVCR_need_check() function checks whether a checkpoint is necessary. The
 * checkpoint system will first check the dirty ratio and variable profile and
 * then makes decision on whether to checkpoint based on the global status of all
 * the variables.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_need_check(void);

/**
 * The ORNVCR_checkpoint() function executes the checkpoint based on the
 * variable profile table.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_checkpoint(void);

/**
 *  The ORNVCR_check_exist () function checks if checkpoint file exists at
 *  predefined locations.
 *
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_check_exist (void);

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
 * @return TRUE upon success.
 * @return FALSE upon errors, and the global variable errno is set to indicate
 * the error; EINVAL: The options argument is not valid.
 */
bool
ORNVCR_restore (int var_address, int size);

#endif /* ORNVCR_INCLUDE_H */

/* vim: set tabstop=4:shiftwidth=4:expandtab:textwidth=78:filetype=C */
