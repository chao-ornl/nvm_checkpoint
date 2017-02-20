/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#ifndef ORNVCR_TYPES_H
#define ORNVCR_TYPES_H

#include <time.h>
#include <sys/time.h>

#ifndef UTHASH_TYPES_H
#define UTHASH_TYPES_H
#include <uthash.h>

typedef struct _chkptScheme {
    int     where_to_checkpoint;
    double  interval_threshold; /* Threshold interval to checkpoint. */
    double  dirty_threshold;    /* threshold dirty ratio to checkpoint. */
} chkptScheme_t;

typedef struct varProfile_t {
    int                 index; /* As in the allocate order; SCR uses the same approach */
    struct timeval      allocate_time;
    int                 latest_checkpoint_time;
    size_t              size;
    int                 type; /* Data type/structure */
    double              dirty_ratio;
    int                 placement; /* Current variable place: stack, DRAM, SSD */
    chkptScheme_t       cScheme;
#if 0
    ORHASH is not integrated yet
    orhash_t            *var_hash; /* This is the data structure for checking dirty ratio,
                                      it is compatible with liborhash. */
#endif
    struct varProfile_t *nextProfile;
    UT_hash_handle hh;
} varProfile_t;

typedef struct _varMonitor {
    varProfile_t    *headProfile;
    varProfile_t    *tailProfile;
    double          dirtyratio;
    time_t          latest_checkpoint_time;
} varMonitor_t;


#endif /* UTHASH_TYPES_H */
#endif /* ORNVCR_TYPES_H */
