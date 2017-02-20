/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#include <errno.h>
#include <stdlib.h>

#include "ORNVCR.h"
#include "uthash.h"

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
ORNVCR_register(varMonitor_t *mon, void* var_address, varProfile_t profile)
{
    return false;
}

bool
ORNVCR_deregister(varMonitor_t *mon, void* var_address)
{
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

