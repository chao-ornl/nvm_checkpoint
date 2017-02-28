
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "ORNVCR.h"
#include "uthash.h"

void hashtable_add_var(varMonitor_t *mon, varProfile_t *profile) {
	varProfile_t *tmp;

	HASH_FIND_INT(mon->hashtableProfile, &(profile->address), tmp);
	if (tmp == NULL) {
		HASH_ADD_INT(mon->hashtableProfile, address, profile);
	} else {
		tmp->index = profile->index;
		tmp->allocate_time = profile->allocate_time;
		tmp->latest_checkpoint_time = profile->latest_checkpoint_time;
		tmp->size = profile->size;
		tmp->type = profile->type;
		tmp->dirty_ratio = profile->dirty_ratio;
		tmp->placement = profile->placement;
		tmp->cScheme = profile->cScheme;
		free(profile);
	}
}

varProfile_t *hashtable_find_var(varMonitor_t *mon, void *address) {
	varProfile_t *tmp;

	HASH_FIND_INT(mon->hashtableProfile, &address, tmp);
	return tmp;
}

void hashtable_delete_var(varMonitor_t *mon, void *address) {
	varProfile_t *tmp;
	HASH_FIND_INT(mon->hashtableProfile, &address, tmp);
	HASH_DEL(mon->hashtableProfile, tmp);
	free(tmp);
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

