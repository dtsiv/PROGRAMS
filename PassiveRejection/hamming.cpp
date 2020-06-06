#include "routines.h"
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

// #define CROSS_CHECK
int hamming() {
    lapack_int i, j, k, n;
    double *b, *x;
    double *dGain;
    double *acopy;
    double *hamm;
    double *hs2;
    int nVelBinsPerChan;
    double *wRe, *wIm;
    double dC = 0.299e9; // meters per seconds
    double dF = 9.4e9; // Hertz (=9400 MHz)
    double dTs = 0.12e-6; // seconds
    double dNT = 220.0e0; // samples per period
    double dNp = 1024.0e0; // # of samples
    double dVeloCoef = dC/2.0e0/dF/dNp/dNT/dTs;

    n = dNp;
    nVelBinsPerChan = 1;

    b = new double [2*n*n];
    x = new double [2*n];
    dGain = new double [n];
    wRe = new double [n*n];
    wIm = new double [n*n];
    hamm = new double [n];
    hs2 = new double [n];
    acopy = new double [n*n];

    /* matrix a definition */
    double dT=1.0e0; // period 1 s
    double C00=1.0e0;
    double dPI=3.14159265e0;
    double xi2=1.0e-3*C00;   // noise @ -30 dB
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
        }
    }
    // array of RHS
    double dNorm = sqrt(1.0e0/n);
    for (i=0; i<n; i++) { // period index
        for (j=0; j<n; j++) { // frequency index
            double dPhi = 2*dPI*i*j/n;
            int idxRe = i + n*(2*j);
            b[idxRe] = dNorm*cos(dPhi);
            int idxIm = i + n*(2*j+1);
            b[idxIm] = dNorm*sin(dPhi);
        }
    }
    // Hamming window
    for (i=0; i<n; i++) { // channel number
        double dPhi=2.0e0*dPI*i/(dNp-1.0e0);
        hamm[i]=21.0e0/46.0e0*cos(dPhi);
        hamm[i]=25.0e0/46.0e0-hamm[i];
    }
#ifdef CROSS_CHECK
    double dSum=0.0e0, dSum2=0.0e0;
    for (i=0; i<n; i++) { // channel number
        dSum+=hamm[i];
        dSum2+=hamm[i]*hamm[i];
    }
    double dFrac=2.0e0*dSum*dSum/dSum2;
    printf("%e\n",10.0e0*log(dFrac)/log(10.0e0));
    return 0;
#endif
    // h^{\dagger} C h
    for (i=0; i<n; i++) { // channel number
        dGain[i]=0.0e0;
        for (j=0; j<n; j++) { // period index
            x[2*j]=0.0e0;   // Re
            x[2*j+1]=0.0e0; // Im
            for (k=0; k<n; k++) { // period index
                int idxRe = k + n*(2*i);
                int idxIm = k + n*(2*i+1);
                x[2*j]+=acopy[j+n*k]*b[idxRe]*hamm[k];
                x[2*j+1]+=acopy[j+n*k]*b[idxIm]*hamm[k];
            }
        }
        for (j=0; j<n; j++) { // period index
            int idxRe = j + n*(2*i);
            int idxIm = j + n*(2*i+1);
            dGain[i]+=x[2*j]*b[idxRe]*hamm[j];
            dGain[i]+=x[2*j+1]*b[idxIm]*hamm[j];
        }
        // (h^{\dagger} s)^2
        double hsh=0.0e0;
        for (k=0; k<n; k++) { // period index
            int idxRe = k + n*(2*i);
            int idxIm = k + n*(2*i+1);
            hsh+=b[idxRe]*b[idxRe]*hamm[k];
            hsh+=b[idxIm]*b[idxIm]*hamm[k];
        }
        dGain[i]=(hsh*hsh)/dGain[i];
        dGain[i]*=2*dNp*C00;
    }

    for (j=n/2; j<n; j++) { // frequency index
        printf("%5.2f\t%15.5e\t\n", 1.0e0*(j-n)*dVeloCoef, 10.0e0*log(dGain[j])/log(10.0e0));
    }
    for (j=0; j<n/2; j++) { // frequency index
        printf("%5.2f\t%15.5e\t\n",1.0e0*j*dVeloCoef, 10.0e0*log(dGain[j])/log(10.0e0));
    }

    delete [] b;
    delete [] x;
    delete [] dGain;
    delete [] acopy;
    delete [] wRe;
    delete [] wIm;
    delete [] hamm;
    delete [] hs2;

    return 0;
}
