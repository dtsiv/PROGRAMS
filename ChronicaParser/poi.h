#ifndef POI_H
#define POI_H

#include <QtCore>
#include <QtSql>
#include <QByteArray>
#include "nr.h"

int poi20191016();
int interferenceSpectrum();
int interferenceMap();
int intfSpectrumRepresentation(
        qint16 *pData,   // raw signal in (qint16,qint16) (Re,Im) complex pairs as obtained from DB field "complexdata"
        unsigned int iDataSize, // 2*Np*NT_; Np - number of periods; NT_ - recorded data samples per period
        unsigned int NT, // NT - number of samples per period, , typically at 20191016 this 200 samples
        unsigned int NT_, // NT_ - recorded data samples per period, typically at 20191016 this is 80 samples
        unsigned int Ntau, // Ntau - pulse duration (samples), typically at 20191016 this is 8 samples
        unsigned int Np // Np - number of periods, typically at 20191016 this is 1024 periods
);
QByteArray dopplerRepresentation(
        qint16 *pData,   // raw signal in (qint16,qint16) (Re,Im) complex pairs as obtained from DB field "complexdata"
        unsigned int iDataSize, // 2*Np*NT_; Np - number of periods; NT_ - recorded data samples per period
        unsigned int iFilteredSize, // in-place array size at filter output: iFilteredN=NT_-Ntau+1; Ntau -pulse duration (samples)
        unsigned int NT, // NT - number of samples per period, , typically at 20191016 this 200 samples
        unsigned int NT_, // NT_ - recorded data samples per period, typically at 20191016 this is 80 samples
        unsigned int Ntau, // Ntau - pulse duration (samples), typically at 20191016 this is 8 samples
        unsigned int Np, // Np - number of periods, typically at 20191016 this is 1024 periods
        int &NFFT
);
void avevar_poi(Vec_I_DP &data, DP &ave, DP &var);
void ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob);


extern int iDopplerFrom,iDopplerTo,iDelayFrom,iDelayTo;
extern double dThreshold,dRelThresh;
extern int nFalseAlarms;
extern double dCarrierF, dTs;
extern int NT,NT_,Np,Ntau;
extern int iPlotSlicePeriod,iPlotSliceBeam;

// extern int iRawStrobFrom, iRawStrobTo;
extern double dMaxVelocity;
extern double dFalseAlarmProb;

extern int idum;

#endif // POI_H
