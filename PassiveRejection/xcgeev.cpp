#include "routines.h"
//==============================================================================
//
//==============================================================================
int xcgeev() {
    lapack_int n = N, lda = LDA, ldvl = LDVL, ldvr = LDVR, info, lwork;
    lapack_complex_float wkopt;
    lapack_complex_float *work;
    /* rwork dimension should be at least 2*n */
    float rwork[2*N];
    lapack_complex_float w[N], vl[LDVL*N], vr[LDVR*N];
    lapack_complex_float Trans[LDA*N];
    lapack_complex_float a[LDA*N];

    /* matrix a definition */
    double Np=N;
    double dT=1.0e0; // period 1 s
    double C00=1.0e0;
    // double xi2=1.0e-8;
    double dPI=3.14159265e0;
    double OmegaMin=2.0e0*dPI/dT/Np;
    double OmegaC=1.0*OmegaMin;

    lapack_complex_float acopy[LDA*N];
    // lapack_complex_float vltr[LDA*N];
    // lapack_complex_float vrtr[LDA*N];

    for (int i=0;i<N;i++){
        for (int j=0;j<N;j++){
            double m=i-j;
            double m2=m*m;
            double dPow=-0.5e0*dT*dT*OmegaC*OmegaC*m2;
            a[i+j*LDA].real=C00*exp(dPow);
            a[i+j*LDA].imag=0.0e0;
            acopy[i+j*LDA].real=a[i+j*LDA].real;
            acopy[i+j*LDA].imag=a[i+j*LDA].imag;
        }
    }

    // lapack_int LAPACKE_cgeev( int matrix_order, char jobvl, char jobvr,
    //                           lapack_int n, lapack_complex_float* a, lapack_int lda,
    //                           lapack_complex_float* w, lapack_complex_float* vl,
    //                           lapack_int ldvl, lapack_complex_float* vr,
    //                           lapack_int ldvr );
    // lapack_int LAPACKE_cgeev_work( int matrix_order, char jobvl, char jobvr,
    //                                lapack_int n, lapack_complex_float* a,
    //                                lapack_int lda, lapack_complex_float* w,
    //                                lapack_complex_float* vl, lapack_int ldvl,
    //                                lapack_complex_float* vr, lapack_int ldvr,
    //                                lapack_complex_float* work, lapack_int lwork,
    //                                float* rwork );

    lwork = -1;
    info = LAPACKE_cgeev_work(LAPACK_COL_MAJOR, 'V', 'V',
                         n, a, lda, w, vl, ldvl, vr, ldvr,
                         &wkopt, lwork, rwork);
    lwork = (lapack_int)wkopt.real;
    work = (lapack_complex_float*)malloc( lwork*sizeof(lapack_complex_float) );
    /* Solve eigenproblem */
    info = LAPACKE_cgeev_work(LAPACK_COL_MAJOR, 'V', 'V',
                         n, a, lda, w, vl, ldvl, vr, ldvr,
                         work, lwork, rwork);
    /* Check for convergence */
    if( info > 0 ) {
            printf( "The algorithm failed to compute eigenvalues.\n" );
            exit( 1 );
    }
    for (int i=0;i<N;i++) {
        printf("%6.2e\t", w[i].real);
    }
    printf("\n");
    return 0;

    // calculate vl*(xi2+w)^-1*vr^T*
    lapack_complex_float Prod[LDA*N];
    for (int i=0;i<N;i++) {
        for (int j=0;j<N;j++) {
            Trans[i+j*LDA].real=vl[i+j*LDA].real;
            Trans[i+j*LDA].imag=0.0e0;
            Prod[i+j*LDA].real=0.0e0;
            Prod[i+j*LDA].imag=0.0e0;
        }
    }
    for (int i=0;i<N;i++) {
        for (int j=0;j<N;j++) {
            // double dCoef=xi2+w[j].real;
            // Trans[i+j*LDA].real*=1.0/dCoef;
            Trans[i+j*LDA].real*=w[j].real;
        }
    }
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<N;j++) { // period number
            for (int k=0;k<N;k++) { // eigenvector number
                Prod[i+j*LDA].real+=Trans[i+k*LDA].real*vr[j+k*LDA].real;
            }
        }
    }
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<N;j++) { // period number
            Trans[i+j*LDA].real=Prod[i+j*LDA].real;
            Trans[i+j*LDA].imag=0.0e0;
             Prod[i+j*LDA].real=0.0e0;
             Prod[i+j*LDA].imag=0.0e0;
#ifdef UNFILTERED
            Trans[i+j*LDA].real=0.0e0;
            Trans[i+j*LDA].imag=0.0e0;
#endif
        }
#ifdef UNFILTERED
        Trans[i+i*LDA].real=1.0e0;
#endif
    }
    double dTrans[N*LDA];
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<N;j++) { // period number
            dTrans[i+j*LDA]=acopy[i+j*LDA].real;
        }
        dTrans[i+i*LDA]=1.0e-6;
    }
    inverse(dTrans,N);
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<N;j++) { // period number
            Trans[i+j*LDA].real=dTrans[i+j*LDA];
            Trans[i+j*LDA].imag=0.0e0;
        }
    }

    // calculate the frequency response
    lapack_complex_float Resp[LDA*NFREQRESP];
    lapack_complex_float Prod1[LDA*NFREQRESP];
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<NFREQRESP;j++) { // doppler frequency of input signal
             Prod1[i+j*LDA].real=0.0e0;
             Prod1[i+j*LDA].imag=0.0e0;
        }
    }
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<NFREQRESP;j++) { // doppler frequency of input signal
            double phi=2*dPI*(1.0e0*j*N/NFREQRESP)*i/N;
            // double phi=2*dPI*(1.0e0*j*N/NFREQRESP)/2;
            Resp[i+j*LDA].real=cos(phi);
            Resp[i+j*LDA].imag=sin(phi);
        }
    }
    for (int i=0;i<N;i++) { // period number
        for (int j=0;j<NFREQRESP;j++) { // doppler frequency of input signal
            for (int k=0;k<N;k++) { // period number
                Prod1[i+j*LDA].real+=Trans[i+k*LDA].real*Resp[k+j*LDA].real-Trans[i+k*LDA].imag*Resp[k+j*LDA].imag;
                Prod1[i+j*LDA].imag+=Trans[i+k*LDA].real*Resp[k+j*LDA].imag+Trans[i+k*LDA].imag*Resp[k+j*LDA].real;
            }
        }
    }
    for (int i=0;i<N;i++){ // period number
        for (int j=0;j<NFREQRESP;j++){ // doppler frequency of input signal
#ifdef WEIGHTED
            double phi=2.0e0*dPI*i/(Np-1);
            double dHamming = 25.0/46.0-21.0/46.0*cos(phi);
            Resp[i+j*LDA].real=Prod1[i+j*LDA].real*dHamming;
            Resp[i+j*LDA].imag=Prod1[i+j*LDA].imag*dHamming;
#else
            Resp[i+j*LDA].real=Prod1[i+j*LDA].real;
            Resp[i+j*LDA].imag=Prod1[i+j*LDA].imag;
#endif
            Prod1[i+j*LDA].real=0.0e0;
            Prod1[i+j*LDA].imag=0.0e0;
        }
    }
    for (int i=0;i<N;i++){ // doppler channel number@analyser
        for (int j=0;j<NFREQRESP;j++){ // dopller frequenci of input signal
            for (int k=0;k<N;k++){ // period number
                double phi=-2.0e0*dPI*i*k/Np;
                Prod1[i+j*LDA].real+=cos(phi)*Resp[k+j*LDA].real-sin(phi)*Resp[k+j*LDA].imag;
                Prod1[i+j*LDA].imag+=cos(phi)*Resp[k+j*LDA].imag+sin(phi)*Resp[k+j*LDA].real;
            }
        }
    }
    double dMax=-1.0e9;
    for (int j=0;j<NFREQRESP;j++){ // doppler frequency of input signal
        for (int i=0;i<N;i++) { // doppler channel @analyser
            double dFreqResp=0.0e0;
            dFreqResp += Prod1[i+j*LDA].real*Prod1[i+j*LDA].real;
            dFreqResp += Prod1[i+j*LDA].imag*Prod1[i+j*LDA].imag;
            dFreqResp/=N*N;
            if (dFreqResp<1.0e-20) dFreqResp=1.0e-20;
            Resp[i+j*LDA].real=10*log(dFreqResp)/log(10.0e0);
            Resp[i+j*LDA].imag=0.0e0;
            dMax=(dMax<Resp[i+j*LDA].real)?Resp[i+j*LDA].real:dMax;
            // dMax=0;
        }
    }
    for (int j=0;j<NFREQRESP;j++){ // doppler frequency of input signal
        printf("%6.4f ",1.0*N*j/NFREQRESP);
        for (int i=0;i<N;i++) { // doppler channel @analyser
            printf("%6.2f ",Resp[i+j*LDA].real-dMax);
        }
        printf("\n");
    }
    // print_matrix( "Frequency responses", n, n, Resp, n );

    /* Free workspace */
    free( (void*)work );
    exit( 0 );

}
