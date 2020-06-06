#include "routines.h"
/*==============================================================================
*
* SUBROUTINE DPBSVX( FACT, UPLO, N, KD, NRHS, AB, LDAB, AFB, LDAFB,
*                    EQUED, S, B, LDB, X, LDX, RCOND, FERR, BERR,
*                    WORK, IWORK, INFO )
*  -- LAPACK driver routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      CHARACTER          EQUED, FACT, UPLO
      INTEGER            INFO, KD, LDAB, LDAFB, LDB, LDX, N, NRHS
      DOUBLE PRECISION   RCOND
*     ..
*     .. Array Arguments ..
      INTEGER            IWORK( * )
      DOUBLE PRECISION   AB( LDAB, * ), AFB( LDAFB, * ), B( LDB, * ),
     $                   BERR( * ), FERR( * ), S( * ), WORK( * ),
     $                   X( LDX, * )
*     ..
*
*  Purpose
*  =======
*
*  DPBSVX uses the Cholesky factorization A = U**T*U or A = L*L**T to
*  compute the solution to a real system of linear equations
*     A * X = B,
*  where A is an N-by-N symmetric positive definite band matrix and X
*  and B are N-by-NRHS matrices.
*
*  Error bounds on the solution and a condition estimate are also
*  provided.
*
*  Description
*  ===========
*
*  The following steps are performed:
*
*  1. If FACT = 'E', real scaling factors are computed to equilibrate
*     the system:
*        diag(S) * A * diag(S) * inv(diag(S)) * X = diag(S) * B
*     Whether or not the system will be equilibrated depends on the
*     scaling of the matrix A, but if equilibration is used, A is
*     overwritten by diag(S)*A*diag(S) and B by diag(S)*B.
*
*  2. If FACT = 'N' or 'E', the Cholesky decomposition is used to
*     factor the matrix A (after equilibration if FACT = 'E') as
*        A = U**T * U,  if UPLO = 'U', or
*        A = L * L**T,  if UPLO = 'L',
*     where U is an upper triangular band matrix, and L is a lower
*     triangular band matrix.
*
*  3. If the leading i-by-i principal minor is not positive definite,
*     then the routine returns with INFO = i. Otherwise, the factored
*     form of A is used to estimate the condition number of the matrix
*     A.  If the reciprocal of the condition number is less than machine
*     precision, INFO = N+1 is returned as a warning, but the routine
*     still goes on to solve for X and compute error bounds as
*     described below.
*
*  4. The system of equations is solved for X using the factored form
*     of A.
*
*  5. Iterative refinement is applied to improve the computed solution
*     matrix and calculate error bounds and backward error estimates
*     for it.
*
*  6. If equilibration was used, the matrix X is premultiplied by
*     diag(S) so that it solves the original system before
*     equilibration.
*
*  Arguments
*  =========
*
*  FACT    (input) CHARACTER*1
*          Specifies whether or not the factored form of the matrix A is
*          supplied on entry, and if not, whether the matrix A should be
*          equilibrated before it is factored.
*          = 'F':  On entry, AFB contains the factored form of A.
*                  If EQUED = 'Y', the matrix A has been equilibrated
*                  with scaling factors given by S.  AB and AFB will not
*                  be modified.
*          = 'N':  The matrix A will be copied to AFB and factored.
*          = 'E':  The matrix A will be equilibrated if necessary, then
*                  copied to AFB and factored.
*
*  UPLO    (input) CHARACTER*1
*          = 'U':  Upper triangle of A is stored;
*          = 'L':  Lower triangle of A is stored.
*
*  N       (input) INTEGER
*          The number of linear equations, i.e., the order of the
*          matrix A.  N >= 0.
*
*  KD      (input) INTEGER
*          The number of superdiagonals of the matrix A if UPLO = 'U',
*          or the number of subdiagonals if UPLO = 'L'.  KD >= 0.
*
*  NRHS    (input) INTEGER
*          The number of right-hand sides, i.e., the number of columns
*          of the matrices B and X.  NRHS >= 0.
*
*  AB      (input/output) DOUBLE PRECISION array, dimension (LDAB,N)
*          On entry, the upper or lower triangle of the symmetric band
*          matrix A, stored in the first KD+1 rows of the array, except
*          if FACT = 'F' and EQUED = 'Y', then A must contain the
*          equilibrated matrix diag(S)*A*diag(S).  The j-th column of A
*          is stored in the j-th column of the array AB as follows:
*          if UPLO = 'U', AB(KD+1+i-j,j) = A(i,j) for max(1,j-KD)<=i<=j;
*          if UPLO = 'L', AB(1+i-j,j)    = A(i,j) for j<=i<=min(N,j+KD).
*          See below for further details.
*
*          On exit, if FACT = 'E' and EQUED = 'Y', A is overwritten by
*          diag(S)*A*diag(S).
*
*  LDAB    (input) INTEGER
*          The leading dimension of the array A.  LDAB >= KD+1.
*
*  AFB     (input or output) DOUBLE PRECISION array, dimension (LDAFB,N)
*          If FACT = 'F', then AFB is an input argument and on entry
*          contains the triangular factor U or L from the Cholesky
*          factorization A = U**T*U or A = L*L**T of the band matrix
*          A, in the same storage format as A (see AB).  If EQUED = 'Y',
*          then AFB is the factored form of the equilibrated matrix A.
*
*          If FACT = 'N', then AFB is an output argument and on exit
*          returns the triangular factor U or L from the Cholesky
*          factorization A = U**T*U or A = L*L**T.
*
*          If FACT = 'E', then AFB is an output argument and on exit
*          returns the triangular factor U or L from the Cholesky
*          factorization A = U**T*U or A = L*L**T of the equilibrated
*          matrix A (see the description of A for the form of the
*          equilibrated matrix).
*
*  LDAFB   (input) INTEGER
*          The leading dimension of the array AFB.  LDAFB >= KD+1.
*
*  EQUED   (input or output) CHARACTER*1
*          Specifies the form of equilibration that was done.
*          = 'N':  No equilibration (always true if FACT = 'N').
*          = 'Y':  Equilibration was done, i.e., A has been replaced by
*                  diag(S) * A * diag(S).
*          EQUED is an input argument if FACT = 'F'; otherwise, it is an
*          output argument.
*
*  S       (input or output) DOUBLE PRECISION array, dimension (N)
*          The scale factors for A; not accessed if EQUED = 'N'.  S is
*          an input argument if FACT = 'F'; otherwise, S is an output
*          argument.  If FACT = 'F' and EQUED = 'Y', each element of S
*          must be positive.
*
*  B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
*          On entry, the N-by-NRHS right hand side matrix B.
*          On exit, if EQUED = 'N', B is not modified; if EQUED = 'Y',
*          B is overwritten by diag(S) * B.
*
*  LDB     (input) INTEGER
*          The leading dimension of the array B.  LDB >= max(1,N).
*
*  X       (output) DOUBLE PRECISION array, dimension (LDX,NRHS)
*          If INFO = 0 or INFO = N+1, the N-by-NRHS solution matrix X to
*          the original system of equations.  Note that if EQUED = 'Y',
*          A and B are modified on exit, and the solution to the
*          equilibrated system is inv(diag(S))*X.
*
*  LDX     (input) INTEGER
*          The leading dimension of the array X.  LDX >= max(1,N).
*
*  RCOND   (output) DOUBLE PRECISION
*          The estimate of the reciprocal condition number of the matrix
*          A after equilibration (if done).  If RCOND is less than the
*          machine precision (in particular, if RCOND = 0), the matrix
*          is singular to working precision.  This condition is
*          indicated by a return code of INFO > 0.
*
*  FERR    (output) DOUBLE PRECISION array, dimension (NRHS)
*          The estimated forward error bound for each solution vector
*          X(j) (the j-th column of the solution matrix X).
*          If XTRUE is the true solution corresponding to X(j), FERR(j)
*          is an estimated upper bound for the magnitude of the largest
*          element in (X(j) - XTRUE) divided by the magnitude of the
*          largest element in X(j).  The estimate is as reliable as
*          the estimate for RCOND, and is almost always a slight
*          overestimate of the true error.
*
*  BERR    (output) DOUBLE PRECISION array, dimension (NRHS)
*          The componentwise relative backward error of each solution
*          vector X(j) (i.e., the smallest relative change in
*          any element of A or B that makes X(j) an exact solution).
*
*  WORK    (workspace) DOUBLE PRECISION array, dimension (3*N)
*
*  IWORK   (workspace) INTEGER array, dimension (N)
*
*  INFO    (output) INTEGER
*          = 0:  successful exit
*          < 0:  if INFO = -i, the i-th argument had an illegal value
*          > 0:  if INFO = i, and i is
*                <= N:  the leading minor of order i of A is
*                       not positive definite, so the factorization
*                       could not be completed, and the solution has not
*                       been computed. RCOND = 0 is returned.
*                = N+1: U is nonsingular, but RCOND is less than machine
*                       precision, meaning that the matrix is singular
*                       to working precision.  Nevertheless, the
*                       solution and error bounds are computed because
*                       there are a number of situations where the
*                       computed solution can be more accurate than the
*                       value of RCOND would suggest.
*
*  Further Details
*  ===============
*
*  The band storage scheme is illustrated by the following example, when
*  N = 6, KD = 2, and UPLO = 'U':
*
*  Two-dimensional storage of the symmetric matrix A:
*
*     a11  a12  a13
*          a22  a23  a24
*               a33  a34  a35
*                    a44  a45  a46
*                         a55  a56
*     (aij=conjg(aji))         a66
*
*  Band storage of the upper triangle of A:
*
*      *    *   a13  a24  a35  a46
*      *   a12  a23  a34  a45  a56
*     a11  a22  a33  a44  a55  a66
*
*  Similarly, if UPLO = 'L' the format of A is as follows:
*
*     a11  a22  a33  a44  a55  a66
*     a21  a32  a43  a54  a65   *
*     a31  a42  a53  a64   *    *
*
*  Array elements marked * are not used by the routine.
*
*==============================================================================*/

// #define RHO_FFT
#ifdef RHO_FFT
double sinc(double dArg) {
    double dEPS=1.0e-8;
    if (dArg<dEPS && dArg>-dEPS) {
        return 1.0e0;
    }
    return (sin(dArg)/dArg);
}
#endif

// #define GAIN

int xdpbsv() {
    /*
    The band storage scheme is illustrated by the following example, when
      N = 6, KD = 2, and UPLO = 'U':

      On entry:                       On exit:

          *    *   a13  a24  a35  a46      *    *   u13  u24  u35  u46
          *   a12  a23  a34  a45  a56      *   u12  u23  u34  u45  u56
         a11  a22  a33  a44  a55  a66     u11  u22  u33  u44  u55  u66

      Similarly, if UPLO = 'L' the format of A is as follows:

      On entry:                       On exit:

         a11  a22  a33  a44  a55  a66     l11  l22  l33  l44  l55  l66
         a21  a32  a43  a54  a65   *      l21  l32  l43  l54  l65   *
         a31  a42  a53  a64   *    *      l31  l42  l53  l64   *    *

      Array elements marked * are not used by the routine.
    */


    // lapack_int LAPACKE_dpbsv( int matrix_order, char uplo, lapack_int n,
    //                           lapack_int kd, lapack_int nrhs, double* ab,
    //                           lapack_int ldab, double* b, lapack_int ldb );
    // lapack_int LAPACKE_dpbsvx_work( int matrix_order, char fact, char uplo,
    //                                 lapack_int n, lapack_int kd, lapack_int nrhs,
    //                                 double* ab, lapack_int ldab, double* afb,
    //                                 lapack_int ldafb, char* equed, double* s,
    //                                 double* b, lapack_int ldb, double* x,
    //                                 lapack_int ldx, double* rcond, double* ferr,
    //                                 double* berr, double* work, lapack_int* iwork );

    char fact = 'N';
    char uplo = 'U';
    double rcond;
    lapack_int i, info, j, kd, ldab, ldafb, ldb, ldx, n, nrhs;
    lapack_int k;
    // lapack_int ifail;
    char cEqued = 'N';
    char *equed = &cEqued;
    double *ab, *afb, *b, *berr, *ferr, *s, *work, *x;
    double dProdRe;
#ifdef GAIN
    double *dGain;
#else
    double *dVelCharact;
    double *dVelChMax;
#endif
    lapack_int* iwork;
    double *acopy;
    int nVelBinsPerChan;
#ifdef GAIN
    nVelBinsPerChan = 1;
#else
    nVelBinsPerChan = 50;
    int nVeloSpan = 100; // channels
#endif
    // double dGainMax;
    // double dGainMin;
    double *wRe, *wIm;
    double dC = 0.299e9; // meters per seconds
    double dF = 9.4e9; // Hertz (=9400 MHz)
    double dTs = 0.12e-6; // seconds
    double dNT = 220.0e0; // samples per period
    double dNp = 1024.0e0; // # of samples
    double dVeloCoef = dC/2.0e0/dF/dNp/dNT/dTs/nVelBinsPerChan;

    n = dNp;
    // number of superdiagonals (same as number of subdiagonals for SB matrix)
    // kd = N-1;
    kd = 250;
    // leading dimension of matrix A_Band
    ldab = kd + 1;
    ldafb = kd + 1;
    ldb = n;
    ldx = n;
    nrhs = 2*nVeloSpan*nVelBinsPerChan;

    ab = new double [ldab*n];
    afb = new double [ldafb*n];
    b = new double [ldb*nrhs];
    berr = new double [nrhs];
    ferr = new double [nrhs];
    s = new double [n];
    work = new double [3*n];
    x = new double [ldx*nrhs];
#ifdef GAIN
    dGain = new double [ldx*nrhs];
#else
    dVelCharact = new double [ldx*nrhs];
    dVelChMax = new double [ldx];
#endif
    wRe = new double [n*n];
    wIm = new double [n*n];

    iwork = new lapack_int[n];

    acopy = new double [n*n];

    /* matrix a definition */
    // double Np=N;
    double dT=1.0e0; // period 1 s
    double C00=1.0e0;
    double dPI=3.14159265e0;
    double xi2=1.0e-3*C00;   // noise @ -30 dB
    // double OmegaMin=2.0e0*dPI/dT/Np;
    double OmegaMax=1.0e0*dPI/dT;
    double OmegaC=0.007e0*OmegaMax;

#ifdef RHO_FFT
    xi2=xi2/dT*sqrt(2*dPI/OmegaC/OmegaC);
    i=n/2;
    for (k=0; k<n; k++) {
        double Sw=0.0e0;
        double Wcurr=2*dPI/dT/n*k-dPI/dT;
        for (j=0; j<n; j++){
            double m=i-j;
            double m2=m*m;
            double dPow=-0.5e0*dT*dT*OmegaC*OmegaC*m2;
            double dRho=C00*exp(dPow);
            if (j==i) dRho+=xi2;
            if (m>kd || m<-kd) dRho=0.0e0;
            Sw+=dT/C00*sqrt(OmegaC*OmegaC/2/dPI)*cos(Wcurr*m*dT)*dRho;
        }
        double m=k-n/2;
        double m2=m*m;
        double dPow=-0.5e0*dT*dT*OmegaC*OmegaC*m2;
        double dRho=C00*exp(dPow);
        if (m>kd || m<-kd) dRho=10.0e-10;
        if (m==0) dRho+=xi2;
        printf("%i\t%e\t%e\n",k-n/2,10.0e0*log(Sw)/log(10.0e0),10.0e0*log(dRho)/log(10.0e0));
    }
    return 0;
#endif

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
            acopy[i+n*j]=0.0e0;
            if (i==j) {
                acopy[i+n*j]+=xi2;
            }
            acopy[i+n*j]+=C00*exp(dPow);
            if (uplo=='U') {
                if (j >= i && j <= n && j <= (i+kd)) {
                    ab[(kd+i-j)+ldab*j] = acopy[i+n*j];
                }
            }
            else if (uplo=='L') {
                if (j >= 0 && j >= (i-kd) && j <= i) {
                    ab[(i-j)+ldab*j] = acopy[i+n*j];
                }
            }
            else {
                return 1;
            }
        }
    }
    // array of RHS
    double dNorm = sqrt(1.0e0/n);
    for (i=0; i<ldb; i++) { // period index
        for (j=0; j<nVeloSpan*nVelBinsPerChan; j++) { // target velocity index
            double dPhi = 2*dPI*i*(j-nVeloSpan*nVelBinsPerChan/2)/n/nVelBinsPerChan;
            int idxRe = i + ldb*(2*j);
            b[idxRe] = dNorm*cos(dPhi);
            int idxIm = i + ldb*(2*j+1);
            b[idxIm] = dNorm*sin(dPhi);
        }
    }
    // solve system of linear equations
    info = LAPACKE_dpbsvx_work(LAPACK_COL_MAJOR, fact, uplo,
                               n, kd, nrhs,
                               ab, ldab, afb,
                               ldafb, equed, s,
                               b, ldb, x,
                               ldx, &rcond, ferr,
                               berr, work, iwork);
    if (info) {
        if (info < 0) {
            printf("LAPACKE_dpbsvx_work: %i-th element had an illigal value\n",-info);
        }
        else if (info > n) {
            printf("LAPACKE_dpbsvx_work: matrix is singular to working precision\n");
        }
        else {
            printf("LAPACKE_dpbsvx_work: leading minor of order %i-th is not positive definite\n", info);
        }
        return 1;
    }
    // printf("LAPACKE_dpbsvx_work finished\n");

    // eigenvalues
    // printf("Vector X:\n");
    for (i=0; i<n; i++) { // channel number
        for (k=0; k<n; k++) { // period index
            double dPhi=-2*dPI*i*k/n;
            wRe[i+n*k] = cos(dPhi);
            wIm[i+n*k] = sin(dPhi);
        }
    }
    for (i=0; i<ldx; i++) { // channel number
#ifndef GAIN
        dVelChMax[i]=0.0e0;
#endif
        // printf("channel %i\n",i);
        for (j=0; j<nVeloSpan*nVelBinsPerChan; j++) { // frequency index
            dProdRe = 0.0e0;
            for (k=0; k<ldx; k++) { // period index
                // double dPhi=-2*dPI*i*k/n;
                int idxRe = k+ldx*(2*j);
                double xRe = x[idxRe];
                int idxIm = k+ldx*(2*j+1);
                double xIm = x[idxIm];
                double dDeltaRe=wRe[i+n*k]*xRe-wIm[i+n*k]*xIm;
                // double dDeltaIm=sin(dPhi)*xRe+cos(dPhi)*xIm;
                dProdRe += dDeltaRe;
            }
            dProdRe *= dNorm;
#ifdef GAIN
            dGain[i+ldx*j] = 2.0e0*dProdRe;
#else
            dProdRe=dProdRe*dProdRe;
            dVelCharact[i+ldx*j] = dProdRe;
            dVelChMax[i]=(dVelChMax[i]>dProdRe)?dVelChMax[i]:dProdRe;
#endif
            // dGainMax = (dGainMax < dGain[i+ldx*j]) ? dGain[i+ldx*j] : dGainMax;
        }
        // dGainMin = (dGainMin > dGain[i+ldx*i]) ? dGain[i+ldx*i] : dGainMin;
    }

#ifdef GAIN
    double dSumCnn = 0.0e0;
    for (i=0;i<n;i++) {
        dSumCnn += acopy[i+n*i];
    }
    for (j=n*nVelBinsPerChan/2; j<n*nVelBinsPerChan; j++) { // frequency index
        printf("%5.2f\t",1.0e0*(j-n*nVelBinsPerChan)*dVeloCoef);
        // for (i=0;i<ldx;i+=100) {
            double dP = dGain[j+ldx*j] * dSumCnn;
            printf("%9.2e\t", 10.0e0*log(dP)/log(10.0e0));
            // printf("%9.2e\t", dP);
        // }
        printf("\n");
    }
    for (j=0; j<n*nVelBinsPerChan/2; j++) { // frequency index
        printf("%5.2f\t",1.0e0*j*dVeloCoef);
        // for (i=0;i<ldx;i+=100) {
            double dP = dGain[j+ldx*j] * dSumCnn;
            printf("%9.2e\t", 10.0e0*log(dP)/log(10.0e0));
            // printf("%9.2e\t", dP);
        // }
        printf("\n");
    }
#else
    for (j=0; j<nVeloSpan*nVelBinsPerChan; j++) { // frequency index
        printf("%5.2f\t",1.0e0*(j-nVeloSpan*nVelBinsPerChan/2)*dVeloCoef);
        for (i=0;i<100;i+=4) {
            double dP = dVelCharact[i+ldx*j] / dVelChMax[i];
            printf("%9.2e\t", 10.0e0*log(dP)/log(10.0e0));
        }
        printf("\n");
    }
#endif

    delete [] ab;
    delete [] afb;
    delete [] b;
    delete [] berr;
    delete [] ferr;
    delete [] s;
    delete [] work;
    delete [] x;
#ifdef GAIN
    delete [] dGain;
#else
    delete [] dVelCharact;
    delete [] dVelChMax;
#endif

    delete [] iwork;

    delete [] acopy;
    delete [] wRe;
    delete [] wIm;

    return 0;
}
