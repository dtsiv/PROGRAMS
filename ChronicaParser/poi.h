#ifndef POI_H
#define POI_H

#include <QtCore>
#include <QtSql>
#include <QByteArray>

int poi();
int poi20190409();
int justDoppler();
int poiRaw();
int poiNoncoher();
int plotSignal();
int plotSignal3D();
QByteArray dopplerRepresentation(qint16 *pData, unsigned int iDataSize, unsigned int iFilteredSize,
                                 unsigned int NT, unsigned int NT_, unsigned int Ntau, unsigned int Np, int &NFFT);

extern int iDopplerFrom,iDopplerTo,iDelayFrom,iDelayTo;
extern double dThreshold,dRelThresh;
extern int nFalseAlarms;
extern double dCarrierF, dTs;
extern int NT,NT_,Np,Ntau;
extern int iPlotSlicePeriod,iPlotSliceBeam;

extern int iRawStrobFrom, iRawStrobTo;
extern double dMaxVelocity;

#endif // POI_H
