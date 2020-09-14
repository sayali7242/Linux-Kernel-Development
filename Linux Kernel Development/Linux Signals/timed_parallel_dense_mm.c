/******************************************************************************
*
* dense_mm.c
*
* This program implements a dense matrix multiply and can be used as a
* hypothetical workload.
*
* Usage: This program takes a single input describing the size of the matrices
*        to multiply. For an input of size N, it computes A*B = C where each
*        of A, B, and C are matrices of size N*N. Matrices A and B are filled
*        with random values.
*
* Written Sept 6, 2015 by David Ferry
******************************************************************************/

#include <stdio.h>  //For printf()
#include <stdlib.h> //For exit() and atoi()
#include <assert.h> //For assert()
#include <time.h>
#include <limits.h>

const int num_expected_args = 2;
const unsigned sqrt_of_UINT32_MAX = 65536;

// The following line can be used to verify that the parallel computation
// gives identical results to the serial computation. If the verficiation is
// successful then the program executes normally. If the verification fails
// the program will terminate with an assertion error.
//#define VERIFY_PARALLEL

int main( int argc, char* argv[] ){

        unsigned time= 1;
        unsigned index, row, col; //loop indicies
        unsigned matrix_size, squared_size;
        double *A, *B, *C;
        #ifdef VERIFY_PARALLEL
        double *D;
        #endif

        if( argc != num_expected_args ){
                printf("Usage: ./timed_parallel_dense_mm <size of matrices> <runtime>\n");
                exit(-1);
        }

        matrix_size = atoi(argv[1]);
        if (argc ==3)
                time = atoi(argv[2]);

        if( matrix_size > sqrt_of_UINT32_MAX ){
                printf("ERROR: Matrix size must be between zero and 65536!\n");
                exit(-1);
        }

        squared_size = matrix_size * matrix_size;

        printf("Generating matrices...\n");

        A = (double*) malloc( sizeof(double) * squared_size );
        B = (double*) malloc( sizeof(double) * squared_size );
        C = (double*) malloc( sizeof(double) * squared_size );
        #ifdef VERIFY_PARALLEL
        D = (double*) malloc( sizeof(double) * squared_size );
        #endif

        for( index = 0; index < squared_size; index++ ){
                A[index] = (double) rand();
                B[index] = (double) rand();
                C[index] = 0.0;
                #ifdef VERIFY_PARALLEL
                D[index] = 0.0;
                #endif
        }

        printf("Multiplying matrices...\n");
        int i;
        long max = 0, min= LONG_MAX, total =0;
        const long bill = 1000000000L;

        for( i=0; i< time; i++)
        {
                struct timespec start, finish;
                clock_gettime(CLOCK_MONOTONIC_RAW, &start);
                #pragma omp parallel for private(col, row, index)
                for( col = 0; col < matrix_size; col++ ){
                        for( row = 0; row < matrix_size; row++ ){
                                for( index = 0; index < matrix_size; index++){
                                        C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
                                }
                        }
                }
                clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
                long start_time = start.tv_sec * bill + start.tv_nsec;
                long finish_time = finish.tv_sec * bill + finish.tv_nsec;
                long time_taken = finish_time - start_time;

                total+= time_taken;
                if (time_taken < min)
                        min = time_taken;
                if (time_taken > max)
                        max = time_taken;
        }

        long ave = total/ time;
        printf("Minimum time: %ld seconds, %ld nanoseconds\n", min/bill, min%bill);
        printf("Maximum time: %ld seconds, %ld nanoseconds\n", max/bill, max%bill);
        printf("Average time: %ld seconds, %ld nanoseconds\n", ave/bill, ave%bill);

        #ifdef VERIFY_PARALLEL
        printf("Verifying parallel matrix multiplication...\n");
        for( col = 0; col < matrix_size; col++ ){
                for( row = 0; row < matrix_size; row++ ){
                        for( index = 0; index < matrix_size; index++){
                        D[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
                        }
                }
        }

        for( index = 0; index < squared_size; index++ )
                assert( C[index] == D[index] );
        #endif //ifdef VERIFY_PARALLEL

        printf("Multiplication done!\n");

        return 0;
}
