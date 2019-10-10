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

//======================================================================================================
//
//======================================================================================================
int poi() {
    int iRet=0;
    QTextStream tsStdOut(stdout);

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    bOk=query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE beam=:beam AND f.filepath LIKE :filepath");
    query.bindValue(":beam",iPlotSliceBeam);
    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    QFile qfDetectionResults("detection.txt");
    if (!qfDetectionResults.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    qfDetectionResults.seek(qfDetectionResults.size());
    QTextStream tsDetectionResults(&qfDetectionResults);
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        qint64 iTimestamp=query.value(rec.indexOf("timestamp")).toLongLong(&bOk);
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        int iDataSize = baSamples.size();

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        int iFilteredN=NT_-Ntau+1; // in-place array size
        int NFFT=0;

        // marmyshka -- tested
        //for (int iDelay=0; iDelay<NT_; iDelay++) {
        //    for (int kDoppler=0; kDoppler<Np; kDoppler++) {
        //        int idx=2*(kDoppler*NT_+iDelay);
        //        double dphi=-2*3.14159265*kDoppler*Np/4/2048;
        //        int iAmpl=(iDelay>=25);iAmpl*=(iDelay<33);
        //        pData[idx]=1000*iAmpl*cos(dphi);
        //        pData[idx+1]=1000*iAmpl*sin(dphi);
        //    }
        //}

        QByteArray baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount, iFilteredN, NT, NT_, Ntau, Np, NFFT);
        if (NFFT!=2048) {
            tsStdOut << "NFFT!=2048" << endl;
            continue;
        }
        if (baDopplerRepresentation.isEmpty()) return 3;
        if (baDopplerRepresentation.size()<2*iFilteredN*Np*sizeof(double)) return 4;
        double *pDopplerData=(double*)baDopplerRepresentation.data();

        iDelayFrom=qBound(0,iDelayFrom,iFilteredN);
        iDelayTo=qBound(0,iDelayTo,iFilteredN);
        iDopplerFrom=qBound(0,iDopplerFrom,NFFT);
        iDopplerTo=qBound(0,iDopplerTo,NFFT);

        QByteArray baSigMod2(baDopplerRepresentation.size(),0);
        double *pMod2=(double *)baSigMod2.data();
        double dMax2=0.0e0;
        int iMax=-1,kMax=-1;
        // search max mod2
        for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
            for (int kDoppler=iDopplerFrom; kDoppler<iDopplerTo; kDoppler++) {
                int idx=2*(kDoppler*iFilteredN+iDelay);
                double dRe=pDopplerData[idx];
                double dIm=pDopplerData[idx+1];
                double dMod2=dRe*dRe+dIm*dIm;
                pMod2[idx]=dMod2;
                if (dMod2>=dMax2) {
                    dMax2=dMod2;
                    iMax=iDelay;
                    kMax=kDoppler;
                }
            }
        }
        if (kMax<0 || iMax<0) {
            tsStdOut << "Error: kMax<0 || iMax<0: " << iStrob << "\t" << beamCountsNum << "\t" << iFilteredN << endl;
            continue;
        }
        dGlobalMax2=(dMax2>dGlobalMax2)?dMax2:dGlobalMax2;
        if (dMax2<dThreshold) continue;
        double dRelThresh2=dRelThresh*dRelThresh*dMax2;
        // tsStdOut << iStrob << "\t" << QString::number(dMax2,'e',10) << "\t" << kMax << "\t" << iMax << endl;

        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // MHz
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // m/s per MHz
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count
        double dDistCoef = dTs*dVLight/2; // m per sample
        QString qsMaxPoint;
        double dVelo=((kMax>NFFT/2)?(kMax-NFFT):kMax)*dVelCoef;
        qsMaxPoint=QString(" max(%1 m, %2 m/s)")
                .arg(iMax*dDistCoef,0,'f',0)
                .arg(dVelo,0,'f',0);

        // result output
        QFile qfData_d("datad.txt");
        if (!qfData_d.open(QIODevice::ReadWrite)) return 3;
        QTextStream tsDataOut_d(&qfData_d);
        qfData_d.resize(0);
        if (0) for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
            int idx=2*(kMax*iFilteredN+iDelay);
            double dRe=pDopplerData[idx];
            double dIm=pDopplerData[idx+1];
            QString qsMod2=QString::number(dRe*dRe+dIm*dIm);
            tsDataOut_d << QString::number(iDelay*dDistCoef) << "\t" << qsMod2 <<  endl;
        }
        if (0) QProcess::execute("gnuplot", QString(
                              "-e_""set terminal png"""
                              "_-e_""set title '%1-%2%3' font 'arial,18'"""
                              "_-e_""set output '%1-%2-d.png'"""
                              "_-e_""set xlabel 'Distance (m)' offset -1,0 font 'arial,18'"""
                              "_-e_""set ylabel 'Arb' font 'arial,18'"""
                              "_-e_""plot 'datad.txt' with linespoints linewidth 1.5 pointtype 6 pointsize 2 notitle""" )
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(qsMaxPoint)
                          .split("_"));
        QFile qfData_v("datav.txt");
        if (!qfData_v.open(QIODevice::ReadWrite)) return 3;
        QTextStream tsDataOut_v(&qfData_v);
        qfData_v.resize(0);
        if (0) for (int kDoppler=qMax(iDopplerFrom,NFFT/2); kDoppler<iDopplerTo; kDoppler++) {
            int kDoppler1=kDoppler-NFFT;
            int idx=2*(kDoppler*iFilteredN+iMax);
            tsDataOut_v << QString::number(kDoppler1*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
        }
        if (0) for (int kDoppler=iDopplerFrom; kDoppler<qMin(iDopplerTo,NFFT/2); kDoppler++) {
            int idx=2*(kDoppler*iFilteredN+iMax);
            tsDataOut_v << QString::number(kDoppler*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
        }
        if (0) QProcess::execute("gnuplot", QString(
                              "-e_""set terminal png"""
                              "_-e_""set title '%1-%2%3' font 'arial,18'"""
                              "_-e_""set logscale y"""
                              "_-e_""set output '%1-%2-v.png'"""
                              "_-e_""set xlabel 'Velocity (m/s)' offset -1,0 font 'arial,18'"""
                              "_-e_""set ylabel 'Arb' font 'arial,18'"""
                              "_-e_""plot 'datav.txt' with lines linewidth 2 notitle"""
                           )
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(qsMaxPoint)
                          .split("_"));
        // if (iPlots++>4) break;
#if 0
        if (1 || dVelo > -100.0 && dVelo < -60.0) {
            tsDetectionResults
                << iStrob << "\t"
                << QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd") << "\t"
                << QString::number(dVelo,'f',1) << "\t"
                << QString::number(iMax*dDistCoef,'f',0) << "\t"
                << QString::number(dMax2,'g',3) << endl;
        }
#endif
    }
    // tsStdOut << "dGlobalMax2 = " << dGlobalMax2 << endl;
    return 0;
}

//======================================================================================================
//
//======================================================================================================
QByteArray dopplerRepresentation(qint16 *pData, unsigned int iArrElemCount, unsigned int iFilteredN,
                                 unsigned int NT, unsigned int NT_, unsigned int Ntau, unsigned int Np, int &NFFT) {
    if (iArrElemCount!=2*NT_*Np || NT_<Ntau || NT<NT_ || iFilteredN!=NT_-Ntau+1) return QByteArray();
    unsigned int i,j,k;
    QTextStream tsStdOut(stdout);

    // loop over periods, in-place calculation
    QByteArray baDataFiltered(2*Np*iFilteredN*sizeof(double),0);
    double *pDataFiltered=(double *)baDataFiltered.data();
    for (k=0; k<Np; k++) {
        // loop over one period (recorded fragment) 0...NT_-1
        for (i=0; i<iFilteredN; i++) {
            qint32 iRe=0,iIm=0; // running cumulants
            int idx;
            for (int itau=0; itau<Ntau; itau++) {
                idx=2*(k*NT_+i+itau); // last runs NT_-Ntau,...,NT_-1
                iRe+=pData[idx]; iIm+=pData[idx+1];
            }
            idx=2*(k*iFilteredN+i); // last i is (NT_-Ntau+1)-1=NT_-Ntau
            pDataFiltered[idx]=iRe; pDataFiltered[idx+1]=iIm;
        }
    }

    NFFT=0;
    for (i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    // if (NFFT!=2048) return QByteArray();

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

//======================================================================================================
//
//======================================================================================================
int plotSignal(void) {
    int iRet=0;
    bool bOk;
    QTextStream                  tsStdOut(stdout);
    if ((iRet=openDatabase())) return iRet+10;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE beam=:beam AND seqnum % 10 = 0");
    query.bindValue(":beam",iPlotSliceBeam);
    if (!query.exec()) return 1;

    QFile qfData("data.txt");
    if (!qfData.open(QIODevice::ReadWrite)) return 3;
    QTextStream tsDataOut(&qfData);
    QSqlRecord rec;
    rec = query.record();
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        qint64 iTimestamp=query.value(rec.indexOf("timestamp")).toLongLong(&bOk);
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        int iDataSize = baSamples.size();
        qfData.resize(0);
        int iOffset = 2*iPlotSlicePeriod*NT_;
        if ((iOffset+2*iPlotSlicePeriod)*sizeof(qint16) > iDataSize) return 4;
        pData+=iOffset;
#pragma GCC diagnostic ignored "-Wsequence-point"
        for (int iSample=0; iSample < NT_; iSample++) {
            tsDataOut << iSample << "\t" << *pData++ << "\t" << *pData++ << endl;
        }
#pragma GCC diagnostic pop
        // int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        // tsStdOut << iStrob << "\t" << iBeam << "\t" << QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd-hh:mm:ss") << endl;
        QProcess::execute("xmgrace", QString("-autoscale none -param data.par -nxy data.txt -hardcopy -hdevice JPEG -printfile %1-%2-%3.jpg")
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(iBeam)
                          .split(" "));
        // QProcess::execute("mirage -f data.jpg");
    }
    return 0;
}

//======================================================================================================
//
//======================================================================================================
int plotSignal3D() {
    int iRet=0;
    QTextStream tsStdOut(stdout);
    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE beam=:beam AND f.filepath=:filepath");
    query.bindValue(":beam",iPlotSliceBeam);
    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        qint64 iTimestamp=query.value(rec.indexOf("timestamp")).toLongLong(&bOk);
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        int iDataSize = baSamples.size();

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        int iFilteredN=NT_-Ntau+1; // in-place array size
        int NFFT=0;

        QByteArray baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount, iFilteredN, NT, NT_, Ntau, Np, NFFT);
        if (NFFT!=2048) {
            tsStdOut << "NFFT!=2048" << endl;
            continue;
        }
        if (baDopplerRepresentation.isEmpty()) return 3;
        if (baDopplerRepresentation.size()<2*iFilteredN*Np*sizeof(double)) return 4;
        double *pDopplerData=(double*)baDopplerRepresentation.data();

        iDelayFrom=qBound(0,iDelayFrom,iFilteredN);
        iDelayTo=qBound(0,iDelayTo,iFilteredN);
        iDopplerFrom=qBound(0,iDopplerFrom,NFFT);
        iDopplerTo=qBound(0,iDopplerTo,NFFT);

        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // MHz
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // m/s per MHz
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count
        double dDistCoef = dTs*dVLight/2; // m per sample

        QFile qfData("data.txt");
        if (!qfData.open(QIODevice::ReadWrite)) return 3;
        qfData.resize(0);
        QTextStream tsDataMap(&qfData);
        double dMax2=0.0e0;
        int iMax=-1,kMax=-1;
        // search max mod2
        for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
            double dMax2bin=0.0e0;
            for (int kDoppler1=-NFFT/2; kDoppler1<NFFT/2; kDoppler1++) {
                int kDoppler=(kDoppler1<0)?(kDoppler1+NFFT):kDoppler1;
                if (kDoppler<iDopplerFrom || kDoppler>=iDopplerTo) {
                    if (kDoppler%PLOT_FREQ_BIN == 0) {
                        tsDataMap << iDelay*dDistCoef << "\t" << kDoppler1*dVelCoef << "\t";
                        tsDataMap << QString::number(1) << endl;
                    }
                    continue;
                }
                int idx=2*(kDoppler*iFilteredN+iDelay);
                double dRe=pDopplerData[idx];
                double dIm=pDopplerData[idx+1];
                double dMod2=dRe*dRe+dIm*dIm;
                dMod2=qMax(dMod2,exp(1.0));
                if (dMod2>=dMax2) {
                    dMax2=dMod2;
                    iMax=iDelay;
                    kMax=kDoppler;
                }
                dMax2bin=(dMax2bin<dMod2)?dMod2:dMax2bin;
                if (kDoppler%PLOT_FREQ_BIN == 0) {
                    tsDataMap << iDelay*dDistCoef << "\t" << kDoppler1*dVelCoef << "\t";
                    tsDataMap << QString::number(log(dMax2bin)) << endl;
                    dMax2bin=0;
                }
            }
            tsDataMap << endl;
        }
        if (kMax<0 || iMax<0) {
            tsStdOut << "Error: kMax<0 || iMax<0: " << iStrob << "\t" << beamCountsNum << "\t" << iFilteredN << endl;
            continue;
        }
        if (dMax2<dThreshold) continue;
        QString qsMaxPoint;
        double dVelo=((kMax>NFFT/2)?(kMax-NFFT):kMax)*dVelCoef;
        qsMaxPoint=QString(" max(%1 m, %2 m/s)")
                .arg(iMax*dDistCoef,0,'f',0)
                .arg(dVelo,0,'f',0);
        QProcess::execute("gnuplot", QString("-e_""set terminal png"""
                                             "_-e_""set title '%1-%2 (%3)' font 'arial,18'"""
                                             "_-e_""set output '%1-%2.png'"""
                                             "_-e_""set view map"""
                                             "_-e_""set xlabel 'Distance (m)' offset -1,0 font 'arial,18'"""
                                             "_-e_""set ylabel 'Velocity (m/s)' font 'arial,18'"""
                                             "_-e_""splot 'data.txt' with pm3d"""
                                             )
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(qsMaxPoint)
                          .split("_"));

    }
    return 0;
}
