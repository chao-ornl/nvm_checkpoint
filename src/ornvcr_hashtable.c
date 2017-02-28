
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "ORNVCR.h"
#include "uthash.h"

void hashtable_add_var(varMonitor_t *mon, varProfile_t *profile) {
	varProfile_t *look_var;

	HASH_FIND_INT(mon->hashtableProfile, &(profile->address), look_var);
	if (look_var == NULL) {
		HASH_ADD_INT(mon->hashtableProfile, address, profile);
	} else {
		look_var->index = profile->index;
		look_var->allocate_time = profile->allocate_time;
		look_var->latest_checkpoint_time = profile->latest_checkpoint_time;
		look_var->size = profile->size;
		look_var->type = profile->type;
		look_var->dirty_ratio = profile->dirty_ratio;
		look_var->placement = profile->placement;
		look_var->cScheme = profile->cScheme;
		free(profile);
	}
}

varProfile_t *hashtable_find_var(varMonitor_t *mon, void *address) {
	varProfile_t *look_var;

	HASH_FIND_INT(mon->hashtableProfile, &address, look_var);
	return look_var;
}

void hashtable_delete_var(varMonitor_t *mon, varProfile_t *profile) {
	HASH_DEL(mon->hashtableProfile, profile);
	free(profile);
}

void hashtable_delete_all(varMonitor_t *mon) {
	varProfile_t *curr, *tmp;

	HASH_ITER(hh, mon->hashtableProfile, curr, tmp) {
		HASH_DEL(mon->hashtableProfile, curr);
		free(curr);
	}
}

void hashtable_list_all(varMonitor_t *mon) {
	varProfile_t *tmp;
	for (tmp = mon->hashtableProfile; tmp != NULL; tmp = (varProfile_t *)(tmp->hh.next)) {
		printf("The variable address: %p\n", tmp->address);
	}
}

int address_sort(varProfile_t *a, varProfile_t *b) {
	return (a->address - b->address);
}

void hashtable_sort_by_address(varMonitor_t *mon) {
	HASH_SORT(mon->hashtableProfile, address_sort);
}

int size_sort(varProfile_t *a, varProfile_t *b) {
	return (a->size - b->size);
}

void hashtable_sort_by_size(varMonitor_t *mon) {
	HASH_SORT(mon->hashtableProfile, size_sort);
}

int dirty_ratio_sort(varProfile_t *a, varProfile_t *b) {
	return (a->dirty_ratio - b->dirty_ratio);
}

void hashtable_sort_by_dirty_ratio(varMonitor_t *mon) {
	HASH_SORT(mon->hashtableProfile, dirty_ratio_sort);
}

int placement_sort(varProfile_t *a, varProfile_t *b) {
	return (a->placement - b->placement);
}

void hashtable_sort_by_placement(varMonitor_t *mon) {
	HASH_SORT(mon->hashtableProfile, placement_sort);
}

