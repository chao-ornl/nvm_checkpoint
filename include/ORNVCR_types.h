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
#include <orhash.h>

#include <pthread.h>

#define LOCALMEM 0
#define LOCALSSD 1
#define PEERSSD  2
#define PFS      3

typedef struct _chkptScheme {
    int     where_to_checkpoint; 
    double  interval_threshold; /* Threshold interval to checkpoint. */
    double  dirty_threshold;    /* threshold dirty ratio to checkpoint. */
} chkptScheme_t;

typedef struct varProfile_t {
    void *              address; /* the address of the variable*/
    int                 index; /* As in the allocate order; SCR uses the same approach */
    struct timeval      allocate_time;
    struct timeval      latest_checkpoint_time;
    size_t              type_size;
    size_t              count; /* Data type/structure */
    size_t              size;
    double              dirty_ratio;
    int                 placement; /* Current variable place: stack, DRAM, SSD */
    chkptScheme_t       cScheme;
    char               chkpt_base_path[80];
#if 1
    orhash_t            *var_hash; /* This is the data structure for checking dirty ratio,
                                      it is compatible with liborhash. */
#endif
    UT_hash_handle hh;
} varProfile_t;

typedef struct _varMonitor {
    varProfile_t    *hashtableProfile;
    int             current_index;
    double          dirtyratio;
    struct timeval  latest_checkpoint_time;
    char *          chkpt_base_path[5]; // the directory that contains checkpoint files, up to 5 levels
    int             current_version;
} varMonitor_t;

extern pthread_t monitor_thread;

struct arg_struct{
    varMonitor_t *mon;
    time_t period;
};

#endif /* UTHASH_TYPES_H */
#endif /* ORNVCR_TYPES_H */
