#include "routines.h"
/* Auxiliary routine: printing a matrix */
void print_matrix( const char* desc, lapack_int m, lapack_int n, double* a, lapack_int lda ) {
        lapack_int i, j;
        printf( "\n %s\n", desc );
        for( i = 0; i < m; i++ ) {
                for( j = 0; j < n; j++ ) printf( " %6.2f", a[i*lda+j] );
                printf( "\n" );
        }
}

/* Auxiliary routine: printing a vector of integers */
void print_int_vector( const char* desc, lapack_int n, lapack_int* a ) {
        lapack_int j;
        printf( "\n %s\n", desc );
        for( j = 0; j < n; j++ ) printf( " %6i", a[j] );
        printf( "\n" );
}

void inverse(double* A, int N_loc) {
    lapack_int m = N_loc, n = N_loc;
    lapack_int lda = N_loc;
    lapack_int *IPIV = new lapack_int[N_loc+1];
    lapack_int LWORK = N_loc*N_loc;
    double *WORK = new double[LWORK];
    lapack_int INFO;

    // LU decomoposition of a general matrix
    // lapack_int LAPACKE_dgetrf_work( int matrix_order, lapack_int m, lapack_int n,
    //                                 double* a, lapack_int lda, lapack_int* ipiv );
    INFO = LAPACKE_dgetrf_work(LAPACK_COL_MAJOR, m, n, A, lda, IPIV);

    // generate inverse of a matrix given its LU decomposition
    // lapack_int LAPACKE_dgetri_work( int matrix_order, lapack_int n, double* a,
    //                                 lapack_int lda, const lapack_int* ipiv,
    //                                 double* work, lapack_int lwork );
    INFO = LAPACKE_dgetri_work(LAPACK_COL_MAJOR, n, A, lda, IPIV, WORK, LWORK);

    delete IPIV;
    delete WORK;
}
