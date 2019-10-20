#include <QtGlobal>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "nr.h"
using namespace std;

#include "sqlmodel.h"
#include "poi.h"

#define PLOT_FREQ_BIN 32

int iDopplerFrom,iDopplerTo,iDelayFrom,iDelayTo;
double dThreshold, dRelThresh;
int nFalseAlarms;
double dCarrierF, dTs;
int NT,NT_,Np,Ntau;
int iPlotSlicePeriod,iPlotSliceBeam;
double dMaxVelocity;
double dFalseAlarmProb;

//====================================================================
//
//====================================================================
void avevar_poi(Vec_I_DP &data, DP &ave, DP &var) {
    DP s;
    int j;

    int n=data.size();
    ave=0.0;
    var=0.0;
    for (j=0;j<n;j++) {
        s=data[j]-ave;
        var += s*s;
    }
    var=var/n;
}
//====================================================================
//
//====================================================================
void ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob) {
    DP var_noise,var_signal,ave_noise,ave_signal;

    int n_noise=data_noise.size();
    int n_signal=data_signal.size();
    if (n_signal!=2) qFatal("n_signal!=2");
    avevar_poi(data_noise,ave_noise,var_noise);
    avevar_poi(data_signal,ave_signal,var_signal);
    f=var_signal/var_noise; // signal-to-noise
    prob = NR::betai(0.5*n_noise,0.5*n_signal,n_noise/(n_noise+n_signal*f));
}
//======================================================================================================
//
//======================================================================================================
QByteArray dopplerRepresentation(qint16 *pData, unsigned int iArrElemCount, unsigned int iFilteredN,
                                 unsigned int NT, unsigned int NT_, unsigned int Ntau, unsigned int Np, int &NFFT) {
    if (iArrElemCount!=2*NT_*Np || NT_<Ntau || NT<NT_ || iFilteredN!=NT_-Ntau+1) return QByteArray();
    unsigned int i,j,k;
    // output text stream for debugging
    QTextStream tsStdOut(stdout);

    // loop over periods, in-place calculation
    // after matched filter the received signal array length is iFilteredN samples
    QByteArray baDataFiltered(2*Np*iFilteredN*sizeof(double),0);
    double *pDataFiltered=(double *)baDataFiltered.data();
    for (k=0; k<Np; k++) {
        // loop over one period (recorded fragment) 0...NT_-1
        for (i=0; i<iFilteredN; i++) { // index within one period
            qint32 iRe=0,iIm=0; // running cumulants
            int idx; // index of input array pData: idx=2*(k*NT_),...,2*(k*NT_+iFilteredN-1)
            idx=2*(k*NT_+i); // k*NT_ - offset of period beginning; i - offset of filter output: i=0,1,...,(NT_-Ntau)
            for (int itau=0; itau<Ntau; itau++) { // summation over pulse duration: itau=0,1,...,(Ntau-1)
                iRe+=pData[idx+2*itau];   // the first array index is 2*(k*NT_)
                iIm+=pData[idx+2*itau+1]; // the last array index is 2*(k*NT_+iFilteredN-1)+2*(Ntau-1)+1 = 2*(k*NT_+NT_)-1
                // Thus within one period we span (2*NT_) pData array elements: 2*(k*NT_)+0, ..., 2*(k*NT_+NT_)-1
            }
            idx=2*(k*iFilteredN+i); // last index i = (NT_-Ntau+1)-1=NT_-Ntau
            // Thus within one period we produce 2*(NT_-Ntau+1) pDataFiltered array elements of type double
            pDataFiltered[idx]=iRe; pDataFiltered[idx+1]=iIm;
        }
    }

    NFFT=0;
    for (i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    if (NFFT!=1024 && NFFT!=2048) return QByteArray();

    QByteArray retVal(2*NFFT*iFilteredN*sizeof(double),0);
    double *pRetData=(double *)retVal.data();
    Vec_DP inData(NFFT*2),vecZeroes(NFFT*2);
    int isign;
    for (i=0; i<NFFT*2; i++) vecZeroes[i]=0.0e0;

    // tsStdOut << "Fourier" << endl;
    // loop over echo delay
    QByteArray baHammingWin(NFFT*sizeof(double),0);
    double *pHammingWin=(double *)baHammingWin.data();
    double dHamA0=25.0/46;
    double dHamA1=21.0/46;
    double dHamW=2.0*3.14159265/(Np-1);
    for (j=0; j<Np; j++) {
        pHammingWin[j]=dHamA0-dHamA1*cos(dHamW*j);
    }
    for (j=Np; j<NFFT; j++) {
        pHammingWin[j]=0.0e0;
    }

    for (i=0; i<iFilteredN; i++) {
        inData=vecZeroes;
        // if (i==10) tsStdOut << i << "\t" << QString::number(inData[2*Np]) << "\t" << QString::number(inData[2*Np+1]) << endl;
        for (j=0; j<Np; j++) {
            int idx=2*(j*iFilteredN+i);
            inData[2*j]=pHammingWin[j]*pDataFiltered[idx];
            inData[2*j+1]=pHammingWin[j]*pDataFiltered[idx+1];
        }
        isign=1;
        NR::four1(inData,isign);
        for (k=0; k<NFFT; k++) {
            int idxW=2*(k*iFilteredN+i);
            pRetData[idxW]=inData[2*k]/NFFT;
            pRetData[idxW+1]=inData[2*k+1]/NFFT;
        }
    }
    return retVal;
}
