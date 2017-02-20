#include <stdio.h>
#include <stdlib.h>

#include "ORNVCR.h"

int
main (int argc, char **argv)
{
    bool            rc;
    varMonitor_t    *mon = NULL;

    rc = ORNVCR_init (&mon);
    if (rc != true || mon == NULL)
    {
        fprintf (stderr, "ERROR: ORNVCR_init() failed\n");
        return EXIT_FAILURE;
    }
    int * test_var=malloc(8*sizeof(int));
    varProfile_t *test_profile=NULL;

    rc=ORNVCR_register(mon, test_var, 8*sizeof(int), sizeof(int), test_profile);
    if(rc=!true)
    {
        fprintf (stderr, "ERROR: ORNVCR_register() failed\n");
        return EXIT_FAILURE;
    }

    rc = ORNVCR_exit (&mon);
    if (rc != true)
    {
        fprintf (stderr, "ERROR: ORNVCR_exit() failed\n");
        return EXIT_FAILURE;
    }

    if (mon != NULL)
    {
        fprintf (stderr, "ERROR: the handle should be NULL\n");
        return EXIT_FAILURE;
    }

    fprintf (stdout, "SUCCESS\n");

    return EXIT_SUCCESS;
}
