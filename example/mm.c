/*
 * Matrix-Matrix Multiplication using row-wise splitting
 * 
 * Uses MPI for parallelization
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/mman.h>
#include<sys/time.h>

//ORCR
#define ORCR 1
#include "ORNVCR.h"

#define ERR_BADORDER    255
#define TAG_INIT      31337
#define TAG_RESULT       42

unsigned long filesize = 10*1024*1024+64;       // matrix size: 1024*1024, 1MB for Ai, 8MB for B, 1MB for Ci, 64bytes for metadata
int m;
int mpiRank = -1, mpiSize = 0;
int timestep = 0;

int getRowCount(int rowsTotal, int mpiRank, int mpiSize);
double matrixMultiply(double *A, double *B, double *C, int n, int n_local, int s, int mpiRank, int mpiSize, char *name, size_t size, char *buf);

int main(int argc, char *argv[]) {

#ifdef ORCR
	MPI_Barrier(MPI_COMM_WORLD);
	//create a monitor structure
	varMonitor_t *mon = (varMonitor_t *)malloc(sizeof(varMonitor_t));
	//init
	ORNVCR_init(mon);
	MPI_Barrier(MPI_COMM_WORLD);
#endif

#ifdef ORCR
	//register mpirank
	ORNVCR_register(mon, &timestep, NULL);
#endif

        int n = 0, n_ubound, n_local, n_sq, i, j;

        double *A, *B, *C;
        int sizeToBeSent;

        int s = 1; // step, block size

        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
        MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

        /* Get n and broadcast it to all processes */
        if (!mpiRank) {
                if (argc != 4) {
                        printf("the command should be:\n");
                        printf("%s matrix-size block-size Frequency\n", argv[0]);
#ifdef ORCR
			ORNVCR_deregister(mon,&timestep);
 			ORNVCR_exit(mon);
#endif
                        MPI_Finalize();

                        return ERR_BADORDER;
                } else {
                        n = atoi(argv[1]);      // matrix size
                        s = atoi(argv[2]);      // block size
                        m = atoi(argv[3]);  // Frequency? 

                        if(((n / mpiSize) % s) != 0) {
#ifdef ORCR
				ORNVCR_deregister(mon,&timestep);
 				ORNVCR_exit(mon);
#endif
                        	MPI_Finalize();
                                return ERR_BADORDER;
                        }
                }

                if (!n) {
                        printf("Order of matrix not supplied, terminating.\n");
                }
        }

        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&s, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (!n) {
#ifdef ORCR
		ORNVCR_deregister(mon,&timestep);
 		ORNVCR_exit(mon);
#endif
                MPI_Finalize();
                return ERR_BADORDER;
        }

        if((n%mpiSize)||((n/mpiSize)%s)){ // if not be able to split evenly
#ifdef ORCR
		ORNVCR_deregister(mon,&timestep);
 		ORNVCR_exit(mon);
#endif
                MPI_Finalize();
                return ERR_BADORDER;
        }

        n_local  = getRowCount(n, mpiRank, mpiSize);
        n_ubound = n * n_local;  /* slave array's upper bound (partial matrix) */
        n_sq     = n * n;    /* master array's upper bound (total matrix) */

        MPI_Barrier(MPI_COMM_WORLD);

        A = (double *) malloc(sizeof(double) * n_ubound);
#ifdef ORCR        
	ORNVCR_register(mon, A, NULL);
#endif

        B = (double *) malloc(sizeof(double) * n_sq );
#ifdef ORCR
	ORNVCR_register(mon, B, NULL);
#endif

        C = (double *) malloc(sizeof(double) * n_ubound);
#ifdef ORCR
	ORNVCR_register(mon, C, NULL);
#endif

        if (timestep == 0) {
                MPI_Barrier(MPI_COMM_WORLD);

                if (!mpiRank) {
                        for (j=1; j<mpiSize; j++) {
                                for (i=0; i<n_ubound; i++) { //Matrix A could be read from a data file
                                        A[i] = (double)(i);
                                }
                                sizeToBeSent = n * getRowCount(n, j, mpiSize);
                                MPI_Send(A, sizeToBeSent, MPI_DOUBLE, j, TAG_INIT, MPI_COMM_WORLD);
                        }
                        for (i=0; i<n_ubound; i++) {
                                A[i] = (double)(i);
                        }
                } else { /* Receive parts of A */
                        MPI_Recv(A, n_ubound, MPI_DOUBLE, 0, TAG_INIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                MPI_Barrier(MPI_COMM_WORLD);

                if (!mpiRank) {
                        for (i=0; i<n_sq; i++) { // Matrix B could be read from a data file
                                B[i] = (double)(i);
                        }
                }

                MPI_Barrier(MPI_COMM_WORLD);
                /* Send B completely to each process */
                MPI_Bcast(B, n*n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MPI_Barrier(MPI_COMM_WORLD);

                /* Let each process initialize C to zero */
                for (i=0; i<n_ubound; i++) {
                        C[i] = 0.0;
                }

                MPI_Barrier(MPI_COMM_WORLD);
        }

#ifdef ORCR
	if(ORNVCR_check_exist(mon) == TRUE){
		//when restore, it should be the same order as these variables are registered
		int ckpt_timestep;
		ORNVCR_restore(varMonitor_t *mon, &ckpt_timestep, sizeof(int));
		//if current timestep less than checkpoint timestep, restore
		if(ckpt_timestep > timestep){
			timestep = ckpt_timestep;
			ORNVCR_restore(varMonitor_t *mon, A, sizeof(n_ubound * sizeof(double)));
			ORNVCR_restore(varMonitor_t *mon, B, sizeof(n_ubound * sizeof(double)));
			ORNVCR_restore(varMonitor_t *mon, C, sizeof(n_ubound * sizeof(double)));
		}		
	}
#endif

    	/* Let each process perform its own multiplications */
    	double iot = matrixMultiply(A, B, C, n, n_local, s, mpiRank, mpiSize, name, filesize, buf);

    	MPI_Barrier(MPI_COMM_WORLD);

    	/* Receive partial results from each slave and could print out matrix C to an output file */
    	if (!mpiRank) {
        	for (i=1; i<mpiSize; i++) {
            		sizeToBeSent = n * getRowCount(n, i, mpiSize);
            		MPI_Recv(C, sizeToBeSent, MPI_DOUBLE, i, TAG_RESULT,
                     		MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        	}
        	printf("Matrix C is generated and collected.\n");
    	} else { /* Send partial results to master */
        	MPI_Send(C, n_ubound, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
    	}

    	MPI_Barrier(MPI_COMM_WORLD);
        free(A);
        free(B);
        free(C);

#ifdef ORCR
	// deregister variables from tracking
    	ORNVCR_deregister(mon,&timestep);
    	ORNVCR_deregister(mon,&A);
    	ORNVCR_deregister(mon,&B);
    	ORNVCR_deregister(mon,&C);
    	//finalize monitor structure
	ORNVCR_exit(mon);
#endif
    	MPI_Finalize();
    	return 0;
}

/*
 * rowsTotal: count of rows in a/b/c
 * mpiRank:   mpi job slaves ID (0,1, 2, ...)
 * mpiSize:   count of mpi job slaves
 *
 */
int getRowCount(int rowsTotal, int mpiRank, int mpiSize) {
    	/* Adjust slack of rows in case rowsTotal is not exactly divisible */
    	return (rowsTotal / mpiSize) + (rowsTotal % mpiSize > mpiRank);
}

//double matrixMultiply(double *a, double *b, double *c, int n, int n_local, int s, int mpiRank, int mpiSize) {
double matrixMultiply(double *a, double *b, double *c, int n, int n_local, int s, int mpiRank, int mpiSize, char *name, size_t size, char *buf) {
    	int i, j, k;
    	int i0, j0, k0;
    	int x = 12345;
    	int l = 1;

    	for (i0=timestep; i0<n_local; i0 += s) {
        
		for (j0=0; j0<n; j0 += s)
            		for (k0=0; k0<n; k0 += s)
                		for (i=i0; ((i < (i0+s)) && (i < n_local)); i++)
                    			for (j=j0; ((j<(j0+s)) && (j0 < n)); j++)
                        			for (k=k0; ((k<(k0+s)) && (k0 < n)); k++)
                            				c[i*n + j] += a[i*n + k] * b[k*n + j];

        	if( (i0 != 0) && (i0 % (m * s)) == 0){ //checkpoint every m blocks
        	        timestep = timestep + m * s;    // m: checkpoint interval (number of blocks between checkpoints), s: size of a block

                	MPI_Barrier(MPI_COMM_WORLD);
        
#ifdef ORCR
			// check if the background process thinks it is right time to checkpoint
			if(ORNVCR_need_check(mon)=TRUE)
				//for actual checkpoint, we may need to call SCR library
				ORNVCR_checkpoint(mon);
#endif		
                	l++;
                	if(mpiRank == (mpiSize -1)) { printf("\n");sync();}
                	MPI_Barrier(MPI_COMM_WORLD);
        	}
    	}
    	return 0;
}
