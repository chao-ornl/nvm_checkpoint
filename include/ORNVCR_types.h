/*
 * Copyright (c) 2017       UT-Battelle, LLC
 *                          All rights reserved.
 */

#ifndef ORNVCR_TYPES_H
#define ORNVCR_TYPES_H

typedef struct _chkptScheme {
    int     where_to_checkpoint;
    double  interval_threshold; /* Threshold interval to checkpoint. */
    double  dirty_threshold;    /* threshold dirty ratio to checkpoint. */
} chkptScheme_t;

typedef struct _varProfile {
    int         index; /* As in the allocate order; SCR uses the same approach */
    int         allocate_time;
    int         latest_checkpoint_time;
    int         size;
    int         type; /* Data type/structure */
    int         dirty_ratio;
    int         placement; /* Current variable place: stack, DRAM, SSD */
    chkptScheme cScheme;
    orhash_t    *var_hash; /* This is the data structure for checking dirty ratio,
                           it is compatible with liborhash. */
    varProfile  *nextProfile;
} varProfile_t;

typedef struct _varMonitor {
    varProfile  *headProfile;
    varProfile  *tailProfile;
    double      dirtyratio;
    time_t      latest_checkpoint_time;
} varMonitor_t;

/* GV: we should avoid global variables as much as possible */
varMonitor monitor

#endif /* ORNVCR_TYPES_H */
