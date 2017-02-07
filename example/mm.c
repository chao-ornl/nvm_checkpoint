/*
 * MatMatMult: Matrix-Matrix Multiplication using row-wise splitting
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

#define ERR_BADORDER    255
#define TAG_INIT      31337
#define TAG_RESULT       42

//SCR
#define SCR 1
#include "scr.h"
#include "test_common.h"

//size_t filesize = 512*1024;
unsigned long filesize = 10*1024*1024+64;       // matrix size: 1024*1024, 1MB for Ai, 8MB for B, 1MB for Ci, 64bytes for metadata

#define handle_error(msg) {printf("%s\nstringerror:%s\n", msg, strerror(errno)); exit(-1);}

#define BASE "/root/scr_cache_base/nvmalloc/"   // SSD mount point

typedef struct _pointer_array{
        void *addr;
        size_t len;
        char name[50];
} pointer_array;
pointer_array parray[100];

int a, b, c; // 0 - DRAM; 1 - SSD
int m;
int t;

int getRowCount(int rowsTotal, int mpiRank, int mpiSize);
//double matrixMultiply(double *A, double *B, double *C, int n, int n_local, int s, int mpiRank, int mpiSize);
double matrixMultiply(double *A, double *B, double *C, int n, int n_local, int s, int mpiRank, int mpiSize, char *name, size_t size, char *buf);

void *ssd_malloc(size_t size, int mpiRank, int matrix);
void ssd_free(void *ptr);

int mpiRank = -1, mpiSize = 0;
int timestep = 0;

int main(int argc, char *argv[]) {
        printf("started!\n");

		/*********************************begin ORNVCR********************************************/
		//create a monitor structure
		varMonitor_t *mon=(varMonitor_t *)malloc(sizeof(varMonitor_t));
		//init
		ORNVCR_init(mon);
		/*********************************end ORNVCR********************************************/

		/*********************************begin ORNVCR********************************************/
		//register mpirank
		ORNVCR_register(mon, &timestep, NULL);
		/*********************************end ORNVCR********************************************/

        int n = 0, n_ubound, n_local, n_sq, i, j;
        // int mpiRank = 0, mpiSize = 1;

        double *A, *B, *C;
        int sizeToBeSent;

        int s = 1; // step, block size

        /*
        * cmd:   mpirun mm n
        *
        */
        printf("before mpi init!\n");
        MPI_Init(&argc, &argv);
        printf("mpi initialized!\n");
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);



        MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
/*
#ifdef SCR
        MPI_Barrier(MPI_COMM_WORLD);
        printf("before SCR init!\n");
        SCR_Init();
        printf("after SCR init!\n");
        MPI_Barrier(MPI_COMM_WORLD);
#endif
*/
        /* Get n and broadcast it to all processes */
        if (!mpiRank) {
                if (argc != 8) {
                        printf("the command should be:\n");
                        printf("%s matrix-size block-size A-place B-place C-place In-turn Frequency\n", argv[0]);
/*#ifdef SCR
                        SCR_Finalize();
#endif*/
                        MPI_Finalize();

                        return ERR_BADORDER;
                } else {
                        n = atoi(argv[1]);      // matrix size
                        s = atoi(argv[2]);      // block size
                        a = atoi(argv[3]);      // matrix A placement (0: DRAM, 1: SSD)
                        b = atoi(argv[4]);      // matrix B placement 
                        c = atoi(argv[5]);      // matrix C placement 
                        t = atoi(argv[6]);  // In-turn? 
                        m = atoi(argv[7]);  // Frequency? 

                        if(((n / mpiSize) % s) != 0) {
/*#ifdef SCR
                                SCR_Finalize();
#endif*/
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
        MPI_Bcast(&a, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&b, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (!n) {
/*#ifdef SCR
                SCR_Finalize();
#endif*/
                MPI_Finalize();
                return ERR_BADORDER;
        }

        if((n%mpiSize)||((n/mpiSize)%s)){ // if not be able to split evenly
/*#ifdef SCR
                SCR_Finalize();
#endif*/
                MPI_Finalize();
                return ERR_BADORDER;
        }

        n_local  = getRowCount(n, mpiRank, mpiSize);
        printf("n_local: %d\n", n_local);
        n_ubound = n * n_local;  /* slave array's upper bound (partial matrix) */
        printf("n_ubound: %d\n", n_ubound);
        n_sq     = n * n;    /* master array's upper bound (total matrix) */
        printf("n_sq: %d\n", n_sq);

        MPI_Barrier(MPI_COMM_WORLD);

        // Create matrix A B and C
        if(a == 0){
                A = (double *) malloc(sizeof(double) * n_ubound);
        } else {
                A = (double *) ssd_malloc(sizeof(double) * n_ubound, mpiRank, 0);
        }
        
       /*********************************begin ORNVCR********************************************/
		ORNVCR_register(mon, A, NULL);
	   /*********************************end ORNVCR********************************************/


        if(b == 0) {
                B = (double *) malloc(sizeof(double) * n_sq );
        } else {
                B = (double *) ssd_malloc(sizeof(double) * n_sq , mpiRank, 1);
        }

		/*********************************begin ORNVCR********************************************/
		ORNVCR_register(mon, B, NULL);
		/*********************************end ORNVCR********************************************/

        if(c == 0) {
                C = (double *) malloc(sizeof(double) * n_ubound);
        } else {
                C = (double *) ssd_malloc(sizeof(double) * n_ubound, mpiRank, 2);
        }

		/*********************************begin ORNVCR********************************************/
		ORNVCR_register(mon, C, NULL);
		/*********************************end ORNVCR********************************************/

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

// #ifdef SCR
//         /* allocate space for the checkpoint data (make filesize a function of rank for some variation) */
//         //filesize = filesize+mpiRank;
//         filesize = n_ubound * sizeof(double)  + n_sq * sizeof(double) + n_ubound * sizeof(double) + mpiRank;
//         //filesize = sizeof(A) + sizeof(B) + sizeof(C) + mpiRank;
//         //printf("size of A: %d\n", sizeof(A));
//         //printf("size of B: %d\n", sizeof(B));
//         //printf("size of C: %d\n", sizeof(C));
//         printf("file size: %d\n", filesize);
//         char *buf = (char*) malloc(filesize);           // buf: a checkpoint file pointer 

//         char name[256];
//         char file[SCR_MAX_FILENAME];
//         sprintf(name, "rank_%d.ckpt", mpiRank);
//         int found_checkpoint = 0;

//         if (SCR_Route_file(name, file) == SCR_SUCCESS) {
//                 if (read_checkpoint(file, &timestep, buf, filesize)){
//                         /* read the file ok, no check that contents are good */
//                         found_checkpoint=1;

//                         fprintf(stderr, "file name: %s, timestep: %d\n", file, timestep);
// //                      if (!check_buffer(buf, filesize, mpiRank, timestep)){
// //                              printf("%d: Invalid valud in buffer\n", mpiRank);
// //                              MPI_Abort(MPI_COMM_WORLD, 1);
// //                              return 1;
// //                      }
//                 }
//         }

//         else if (mpiRank == 0) {
//                 printf("Scr_Route_file failed during restart attempt\n");
//         }

//         /* determine whether all tasks successfully read their checkpoint file */
//         int all_found_checkpoint = 0;
//         MPI_Allreduce(&found_checkpoint, &all_found_checkpoint, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
//         if (!all_found_checkpoint && mpiRank == 0) {
//                 printf("At least one rank (perhaps all) did not find its checkpoint\n");
//         }

//         /* check that everyone is at the same timestep */
//         int timestep_and, timestep_or;
//         MPI_Allreduce(&timestep, &timestep_and, 1, MPI_INT, MPI_BAND, MPI_COMM_WORLD);
//         MPI_Allreduce(&timestep, &timestep_or,  1, MPI_INT, MPI_BOR,  MPI_COMM_WORLD);

//         if (timestep_and != timestep_or) {
//                 printf("%d: Timesteps don't agree: timestep %d\n", mpiRank, timestep);
//                 return 1;
//         }

//         if (timestep == 0){
//                 /* make up some data for the next checkpoint */
//                 init_buffer(buf, filesize, mpiRank, timestep);
//         }
//         else{
//                 // unpack 
//                 //memcpy(A, buf, sizeof(A));
//                 //memcpy(B, buf+sizeof(A), sizeof(B));
//                 //memcpy(C, buf+sizeof(A)+sizeof(B), sizeof(C));
//                 memcpy(A, buf, n_ubound);
//                 memcpy(B, buf+n_ubound, n_sq);
//                 memcpy(C, buf+n_ubound+n_sq, n_ubound);
// #if 0   
//                 memcpy(A, buf, 1024*1024);
//                 memcpy(B, buf+1024*1024, 8*1024*1024);
//                 memcpy(C, buf+9*1024*1024, 1024*1024);
// #endif
//         }
// #endif

        /*********************************begin ORNVCR********************************************/
		//replace the above code to ORNNCR calls.
		if(ORNVCR_check_exist (mon)==TRUE){
			//when restore, it should be the same order as these variables are registered
			int ckpt_timestep;
			ORNVCR_restore (varMonitor_t *mon, &ckpt_timestep, sizeof(int));
			//if current timestep less than checkpoint timestep, restore
			if(ckpt_timestep>timestep){
				timestep=ckpt_timestep;
				ORNVCR_restore (varMonitor_t *mon, A, sizeof(n_ubound * sizeof(double)));
				ORNVCR_restore (varMonitor_t *mon, B, sizeof(n_ubound * sizeof(double)));
				ORNVCR_restore (varMonitor_t *mon, C, sizeof(n_ubound * sizeof(double)));
			}
			
		}

		/*********************************end ORNVCR********************************************



    /* Let each process perform its own multiplications */
//    double iot = matrixMultiply(A, B, C, n, n_local, s, mpiRank, mpiSize);
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
    if(a == 0){
                free(A);
    } else {
                ssd_free(A);
    }

    if(b == 0) {
                free(B);
    } else {
                ssd_free(B);
    }

    if(c == 0) {
                free(C);
    } else {
                ssd_free(C);
    }

/*#ifdef SCR
        if (buf != NULL ){
                free(buf);
                buf = NULL;
        }
        SCR_Finalize();
#endif*/
	/*********************************begin ORNVCR********************************************
	*deregister variables from tracking*******************************************************/
    ORNVCR_deregister(mon,&timestep);
    ORNVCR_deregister(mon,&A);
    ORNVCR_deregister(mon,&B);
    ORNVCR_deregister(mon,&C);
    //finalize monitor structure
    ORNVCR_exit(mon)
    /*********************************begin ORNVCR********************************************/
    MPI_Finalize();
    return 0;
}

/*
 * rowsTotal: count of rows in a/b/c
 * mpiRank:   mpi job slaves ID (0,1, 2, ...)
 * mpiSize:   count of mpi job slaves
 *
 */
int getRowCount(int rowsTotal, int mpiRank, int mpiSize) {
    /* Adjust slack of rows in case rowsTotal is not exactly divisible */
    return (rowsTotal / mpiSize) + (rowsTotal % mpiSize > mpiRank);
}

// #ifdef SCR
// int scr_pack(double *a, double *b, double *c, int n_ubound, int n_sq, char *buf)
// {
//   //memcpy(buf, a, sizeof(a));
//   //memcpy(buf+sizeof(a), b, sizeof(b));
//   //memcpy(buf+sizeof(a)+sizeof(b), c, sizeof(c));
//         memcpy(buf, a, n_ubound);
//         memcpy(buf+n_ubound, b, n_sq);
//         memcpy(buf+n_ubound+n_sq, c, n_ubound);

// #if 0 
//         memcpy(buf, a, 1024*1024);
//         memcpy(buf+1024*1024, b, 8*1024*1024);
//         memcpy(buf+9*1024*1024, b, 1024*1024);
// #endif

//         return 0;
// }

// int scr_checkpoint(char *name, char *buf, size_t size, int rank, int in_order){
//         int valid = 0;
//         int rc;
//         char file[SCR_MAX_FILENAME];
//         int x = 12345;

//         fprintf(stderr, "SCR checkpoint begin (mpirank: %d)\n", rank);

//         /* instruct SCR we are starting the next checkpoint */
//         SCR_Start_checkpoint();

//         /* get the file name to write out our checkpoint file to */
//         SCR_Route_file(name, file);

//         /* open the file and write the checkpoint */
//         int fd_me = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

//         if (fd_me > 0){
//                 valid = 1;

//                 /*************************************************
//                  * MPI processes checkpoint in order 
//                  *************************************************/
//                 if (in_order){
//                         if(mpiRank == 0) {
//                                 printf("0 by mpirank 0\n");
//                                 sync();
//                                // SCR_Checkpoint(...);
//                                 rc = write_checkpoint(fd_me, timestep, buf, size);
//                                 MPI_Send(&x, 1, MPI_INT, mpiRank + 1, 0, MPI_COMM_WORLD);
//                         } else if(mpiRank == (mpiSize -1)){
//                                 MPI_Recv (&x, 1, MPI_INT, mpiRank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                                 printf("0 by mpirank %d\n", mpiRank);
//                                 sync();
//                                 // SCR_Checkpoint(...);
//                                 rc = write_checkpoint(fd_me, timestep, buf, size);
//                         } else {
//                                MPI_Recv (&x, 1, MPI_INT, mpiRank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                                 printf("0 by mpirank %d\n", mpiRank);
//                                 sync();

//                                 // SCR_Checkpoint(...);
//                                 rc = write_checkpoint(fd_me, timestep, buf, size);
//                                 MPI_Send(&x, 1, MPI_INT, mpiRank + 1, 0, MPI_COMM_WORLD);
//                         }
//                 }

//                 /*************************************************
//                  * MPI processes compete to checkpoint  
//                  *************************************************/
//                 else{
//                         /* write the checkpoint data */
//                         rc = write_checkpoint(fd_me, timestep, buf, size);
//                 }

//                 if (rc < 0 ){
//                         valid = 0;
//                 }

//                 /* force the data to storage */
//                 rc = fsync (fd_me);
//                 if (rc < 0){
//                         valid = 0;
//                 }
//         }

//         SCR_Complete_checkpoint(valid);
//         fprintf(stderr, "SCR checkpoint end (mpirank: %d)\n", rank);
//         fflush (stdout);
//         /*
//         if ( rank == 0){
//                 printf("scr_checkpoint finished (rank=%d), %d.\n", rank, timestep);
//         //              printf("Completed checkpoint %d.\n", timestep);
//                 fflush (stdout);
//         }
//         */

//         return 0;
// }
// #endif

//double matrixMultiply(double *a, double *b, double *c, int n, int n_local, int s, int mpiRank, int mpiSize) {
double matrixMultiply(double *a, double *b, double *c, int n, int n_local, int s, int mpiRank, int mpiSize, char *name, size_t size, char *buf) {
    int i, j, k;
    int i0, j0, k0;
    int x = 12345;
    int l = 1;

    for (i0=timestep; i0<n_local; i0 += s) {
      //printf("timestep: %d\n", timestep);
        for (j0=0; j0<n; j0 += s) {
            for (k0=0; k0<n; k0 += s) {
              //printf("i0: %d, j0: %d, k0: %d\n", i0, j0, k0);
                for (i=i0; ((i < (i0+s)) && (i < n_local)); i++) {
                    for (j=j0; ((j<(j0+s)) && (j0 < n)); j++) {
                        for (k=k0; ((k<(k0+s)) && (k0 < n)); k++) {
                            c[i*n + j] += a[i*n + k] * b[k*n + j];
                        }
                    }
                }
            }
        }

        if( (i0 != 0) && (i0 % (m * s)) == 0){ //checkpoint every m blocks
//              printf("i0: %d\n", i0);
                timestep = timestep + m * s;    // m: checkpoint interval (number of blocks between checkpoints), s: size of a block
//              printf("timestep: %d\n", timestep);

                MPI_Barrier(MPI_COMM_WORLD);
/*#ifdef SCR
                scr_pack(a, b, c, n*n_local, n*n, buf);
                scr_checkpoint(name, buf, size, mpiRank, t);
#endif*/
        
        /*********************************begin ORNVCR********************************************
		*replace the above code to ORNNCR calls.
		*check if the background process thinks it is right time to checkpoint********************/
		if(ORNVCR_need_check(mon)=TRUE)
			//for actual checkpoint, we may need to call SCR library
			ORNVCR_checkpoint(mon);
		
		/********************************end ORNVCR**********************************************/

                l++;
                if(mpiRank == (mpiSize -1)) { printf("\n");sync();}
                MPI_Barrier(MPI_COMM_WORLD);
        }
//      sleep(1);
    }
    return 0;
}

void *ssd_malloc(size_t size, int mpiRank, int matrix)
{
        struct timeval ct;
        int fd;
        char fname[100] = {'\0', '\n'};
        char exts[20];
        char extus[10];
        char ext[20];
        char rank[20];

        sprintf(ext, "%d", matrix);

        strcat(fname,"");
        strcat(fname, BASE);
        strcat(fname, ext);

        sprintf(rank, "-%d", mpiRank);
        if(matrix != 1) strcat(fname,rank);

        fd = open(fname, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        int fzero = open("/dev/zero", O_RDONLY);
        if(fd == -1 || fzero == -1)
                handle_error("error when openning file");

        int bs = 1048576;
        char data[1048576];
        int count = size/bs;

        while(count > 0)
        {
                read(fzero, data, bs);
                write(fd, data, bs);
                count --;
        }
        write(fd, data, size%bs);
        close(fzero);

        sleep(3);
        int tmp = close(fd);
        if(tmp == -1)
                handle_error("error when closing file");
        sync();
        sleep(20);

        int index = 0;
        while((access(fname, F_OK)) && (index<100)){
                sleep(10);
               index++;
        }

        struct stat sb;
        off_t offset;
        size_t length;

        fd = open(fname, O_RDWR);
        if(fd == -1)
                handle_error("SECOND: error when openning file");

        if(fstat(fd, &sb) == -1)
                handle_error("error when querying stats of file");

        offset = 0 & ~(sysconf(_SC_PAGE_SIZE) -1);
        length = sb.st_size;
        void* addr = mmap(NULL, length, PROT_WRITE | PROT_READ, MAP_SHARED, fd, offset);
        if(addr == MAP_FAILED)
                handle_error("failed mmap");

        parray[fd].addr = addr;
        parray[fd].len = length;
        strcpy(parray[fd].name, fname);
        return addr;
}

void ssd_free(void *ptr)
{
        int i;
        for(i=0; i<100; i++)
        {
                if(parray[i].addr == ptr)
                {
                        munmap(parray[i].addr, parray[i].len);
                        close(i);

                        return;
                }
        }
        return;
}
