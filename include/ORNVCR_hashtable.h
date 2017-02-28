
#ifndef ORNVCR_HASHTABLE_H
#define ORNVCR_HASHTABLE_H

#include <stdbool.h>
#include "ORNVCR_types.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "ORNVCR.h"
#include "uthash.h"

void hashtable_add_var(varMonitor_t *mon, varProfile_t *profile);

varProfile_t *hashtable_find_var(varMonitor_t *mon, void *address);

void hashtable_delete_var(varMonitor_t *mon, varProfile_t *profile);

void hashtable_delete_all(varMonitor_t *mon);

void hashtable_list_all(varMonitor_t *mon);

int address_sort(varProfile_t *a, varProfile_t *b);

void hashtable_sort_by_address(varMonitor_t *mon);

void hashtable_sort_by_size(varMonitor_t *mon);

void hashtable_sort_by_dirty_ratio(varMonitor_t *mon);

void hashtable_sort_by_placement(varMonitor_t *mon);

#endif

