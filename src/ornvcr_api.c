/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#include <stdbool.h>

#include "ORNVCR.h"

bool
ORNVCR_init(void)
{
    return false;
}

bool
ORNVCR_exit(void)
{
    return false;
}

bool
ORNVCR_register(void* var_address, varProfile_t profile)
{
    return false;
}

bool
ORNVCR_deregister(void* var_address)
{
    return false;
}

bool
ORNVCR_get_profile(int var_address, varProfile_t *profile)
{
    return false;
}

bool
ORNVCR_set_profile(int var_address, varProfile_t profile)
{
    return false;
}

bool
ORNVCR_need_check(void)
{
    return false;
}

bool
ORNVCR_checkpoint(void)
{
    return false;
}

bool
ORNVCR_check_exist (void)
{
    return false;
}

bool
ORNVCR_restore (int var_address, int size)
{
    return false;
}

