#include "routines.h"
/*************************************************************************
Purpose:
========
 DSBEV computes all the eigenvalues and, optionally, eigenvectors of
 a real symmetric band matrix A.
Parameters
==========
[in]	JOBZ
          JOBZ is CHARACTER*1
          = 'N':  Compute eigenvalues only;
          = 'V':  Compute eigenvalues and eigenvectors.
[in]	UPLO
          UPLO is CHARACTER*1
          = 'U':  Upper triangle of A is stored;
          = 'L':  Lower triangle of A is stored.
[in]	N
          N is INTEGER
          The order of the matrix A.  N >= 0.
[in]	KD
          KD is INTEGER
          The number of superdiagonals of the matrix A if UPLO = 'U',
          or the number of subdiagonals if UPLO = 'L'.  KD >= 0.
[in,out]	AB
          AB is DOUBLE PRECISION array, dimension (LDAB, N)
          On entry, the upper or lower triangle of the symmetric band
          matrix A, stored in the first KD+1 rows of the array.  The
          j-th column of A is stored in the j-th column of the array AB
          as follows:
          if UPLO = 'U', AB(kd+1+i-j,j) = A(i,j) for max(1,j-kd)<=i<=j;
          if UPLO = 'L', AB(1+i-j,j)    = A(i,j) for j<=i<=min(n,j+kd).

          On exit, AB is overwritten by values generated during the
          reduction to tridiagonal form.  If UPLO = 'U', the first
          superdiagonal and the diagonal of the tridiagonal matrix T
          are returned in rows KD and KD+1 of AB, and if UPLO = 'L',
          the diagonal and first subdiagonal of T are returned in the
          first two rows of AB.
[in]	LDAB
          LDAB is INTEGER
          The leading dimension of the array AB.  LDAB >= KD + 1.
[out]	W
          W is DOUBLE PRECISION array, dimension (N)
          If INFO = 0, the eigenvalues in ascending order.
[out]	Z
          Z is DOUBLE PRECISION array, dimension (LDZ, N)
          If JOBZ = 'V', then if INFO = 0, Z contains the orthonormal
          eigenvectors of the matrix A, with the i-th column of Z
          holding the eigenvector associated with W(i).
          If JOBZ = 'N', then Z is not referenced.
[in]	LDZ
          LDZ is INTEGER
          The leading dimension of the array Z.  LDZ >= 1, and if
          JOBZ = 'V', LDZ >= max(1,N).
[out]	WORK
          WORK is DOUBLE PRECISION array, dimension (max(1,3*N-2))
[out]	INFO
          INFO is INTEGER
          = 0:  successful exit
          < 0:  if INFO = -i, the i-th argument had an illegal value
          > 0:  if INFO = i, the algorithm failed to converge; i
                off-diagonal elements of an intermediate tridiagonal
                form did not converge to zero.
***************************************************************************/
int xdsbev() {
    char uplo = 'U';
    // double eerrbd, eps;
    lapack_int i;
    // lapack_int ifail;
    lapack_int info;
    lapack_int j;
    // number of superdiagonals (same as number of subdiagonals for SB matrix)
    lapack_int kd = 7;
    // leading dimension of matrix A_Band
    lapack_int ldab = kd + 1;
    lapack_int n = 1024;
    lapack_int ldz = n;
    double *ab = new double [ldab*n];
    double *acopy = new double [n*n];
    double *rcondz = new double [n];
    double *w = new double [n];
    double *work = new double [3*n-2];
    double *z = new double [ldz*n];
    double *zerrbd = new double [n];

    /* matrix a definition */
    // double Np=n;
    double dT=1.0e0; // period 1 s
    double C00=1.0e0;
    double xi2=1.0e-4;
    double dPI=3.14159265e0;
    // double OmegaMin=2.0e0*dPI/dT/Np;
    double OmegaMax=2.0e0*dPI/dT;
    double OmegaC=0.1*OmegaMax;


    //====================================================================
    // If (uplo=='U') Then
    //     Read (nin, *)((ab(kd+1+i-j,j),j=i,min(n,i+kd)), i=1, n)
    // Else If (uplo=='L') Then
    //     Read (nin, *)((ab(1+i-j,j),j=max(1,i-kd),i), i=1, n)
    // End If
    //====================================================================
    for (i=0; i<n; i++){
        for (j=0; j<n; j++){
            double m=i-j;
            double m2=m*m;
            double dPow=-0.5e0*dT*dT*OmegaC*OmegaC*m2;
            acopy[i+j*n]=0.0e0;
            acopy[i+i*n]=1.0e0;
            acopy[i+j*n]+=C00*exp(dPow)/xi2;
            if (uplo=='U') {
                if (j >= i && j <= n && j <= (i+kd)) {
                    ab[(kd+i-j)+j*ldab] = acopy[i+j*n];
                }
            }
            else if (uplo=='L') {
                if (j >= 0 && j >= (i-kd) && j <= i) {
                    ab[(i-j)+j*ldab] = acopy[i+j*n];
                }
            }
            else {
                return 1;
            }
        }
    }

    // Solve the band symmetric eigenvalue problem
    // lapack_int LAPACKE_dsbev_work( int matrix_order, char jobz, char uplo,
    //                                lapack_int n, lapack_int kd, double* ab,
    //                                lapack_int ldab, double* w, double* z,
    //                                lapack_int ldz, double* work );
    info = LAPACKE_dsbev_work(LAPACK_COL_MAJOR, 'V', uplo,
                              n, kd, ab,
                              ldab, w, z,
                              ldz, work);
    if (info != 0) {
        if (info < 0) {
            printf("LAPACKE_dsbev_work failed: %i-th argument illegal\n", -info);
        }
        else {
            printf("LAPACKE_dsbev_work failed: %i-th "
                   "off-diagonal elements of an intermediate tridiagonal "
                   "form did not converge to zero\n", info);
        }
        return 2;
    }
    // eigenvalues
    // printf("Eigenvalues:\n");
    for (i=0; i<n; i++) {
        printf("%6.2e\n", w[i]);
    }
    // printf("\n");

    delete[] ab;
    delete[] acopy;
    delete[] rcondz;
    delete[] w;
    delete[] work;
    delete[] z;
    delete[] zerrbd;
    return 0;
}
