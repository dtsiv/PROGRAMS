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

int iRawStrobFrom, iRawStrobTo;

//======================================================================================================
//
//======================================================================================================
int poiRaw() {
    int iRet=0;
    double dGlobalMax2=0.0e0;
    int iMax_g=0;
    int kMax_g=0;

    QTextStream tsStdOut(stdout);
    // tsStdOut << "Inside primaryProc" << endl;

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE seqnum BETWEEN :iRawStrobFrom AND :iRawStrobTo"
                    " AND beam=:beam"
                    " AND f.filepath=:filepath");
    query.bindValue(":beam",iPlotSliceBeam);
    query.bindValue(":filepath",qsGetFileName());
    query.bindValue(":iRawStrobFrom",iRawStrobFrom);
    query.bindValue(":iRawStrobTo",iRawStrobTo);
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    // tsStdOut << "size: " << query.size() << endl;
    // int iPlots=0;
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

#if 0
        //============= marmyshka -- tested
        double dVelo=600.0; // m/s
        int kDopplFreq = 2*dVelo*1.0e10/2.99e8*(dTs*1.0e-6*NT)*2048;
        double dStrobTime=Np*NT; // samples
        
        for (int iDelay=0; iDelay<NT_; iDelay++) {
            for (int kDoppler=0; kDoppler<Np; kDoppler++) {
                int idx=2*(kDoppler*NT_+iDelay);
                double dphi=-2*3.14159265*kDoppler*kDopplFreq/2048;
                double dTargetWay=(iStrob-iRawStrobFrom)*dStrobTime*dTs*1.0e-6*dVelo; // m
                int iTargetDelay=(500.0e0+dTargetWay)*2/2.99e8/(dTs*1.0e-6); // samples
        
                int iAmpl=0;
                if (iDelay>=iTargetDelay && iDelay<iTargetDelay+Ntau) iAmpl=1;
                pData[idx]=1000*iAmpl*cos(dphi);
                pData[idx+1]=1000*iAmpl*sin(dphi);
            }
        }
        //===============
#endif

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
        QByteArray baSigMod2(baDopplerRepresentation.size(),0);
        double *pMod2=(double *)baSigMod2.data();
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
                pMod2[idx]=dMod2;
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
        if (dMax2>dGlobalMax2) {
            dGlobalMax2=dMax2;
            iMax_g=iMax;
            kMax_g=kMax;
        }

        QProcess::execute("gnuplot", QString("-e_""set terminal png"""
                                             "_-e_""set title '%1-%2-%3' font 'arial,18'"""
                                             "_-e_""set output '%1-%2-%3.png'"""
                                             "_-e_""set view map"""
                                             "_-e_""set xlabel 'Distance (m)' offset -1,0 font 'arial,18'"""
                                             "_-e_""set ylabel 'Velocity (m/s)' font 'arial,18'"""
                                             "_-e_""splot 'data.txt' with pm3d"""
                                             )
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(iBeam)
                          .split("_"));

    }
    tsStdOut << "dGlobalMax2 = " << log(dGlobalMax2) << "\tiMax_g=" << iMax_g << "\tkMax_g=" << kMax_g << endl;
    return 0;
}

//======================================================================================================
//
//======================================================================================================
int poiNoncoher() {
    int iRet=0;
    double dGlobalMax2=0.0e0;
    int iMax_g=0;
    int kMax_g=0;

    QTextStream tsStdOut(stdout);
    // tsStdOut << "Inside primaryProc" << endl;

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE beam=0");
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    // tsStdOut << "size: " << query.size() << endl;
    // int iPlots=0;
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

        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // MHz
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // m/s per MHz
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count
        double dDistCoef = dTs*dVLight/2; // m per sample

        QFile qfData("data.txt");
        if (!qfData.open(QIODevice::ReadWrite)) return 3;
        qfData.resize(0);
        QTextStream tsData(&qfData);
        for (int iDelay=0; iDelay<NT_; iDelay++) {
            double dMod2=0.0e0;
            for (int kPeriod=0; kPeriod<Np; kPeriod++) {
                int idx=2*(kPeriod*NT_+iDelay);
                double dRe=pData[idx];
                double dIm=pData[idx+1];
                dMod2+=dRe*dRe+dIm*dIm;
            }
            dMod2=dMod2*1.0e0/Np;
            tsData << dDistCoef*iDelay << "\t" << dMod2 << endl;
        }
        QProcess::execute("gnuplot", QString("-e_""set terminal png"""
                                             "_-e_""set title '%1-%2-%3' font 'arial,18'"""
                                             "_-e_""set output '%1-%2-%3.png'"""
                                             "_-e_""set xlabel 'Distance (m)' offset -1,0 font 'arial,18'"""
                                             "_-e_""set ylabel 'Arb' font 'arial,18'"""
                                             "_-e_""plot 'data.txt'"""
                                             )
                          .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                          .arg(iStrob)
                          .arg(iBeam)
                          .split("_"));

    }
    return 0;
}


//################################################################################################################
#if 0
// result output
QFile qfData_d("data_d.txt");
if (!qfData_d.open(QIODevice::ReadWrite)) return 3;
QTextStream tsDataOut_d(&qfData_d);
qfData_d.resize(0);
for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
    int idx=2*(kMax*iFilteredN+iDelay);
    double dRe=pDopplerData[idx];
    double dIm=pDopplerData[idx+1];
    QString qsMod2=QString::number(qMax(1.0e-6,dRe*dRe+dIm*dIm));
    tsDataOut_d << QString::number(iDelay*dDistCoef) << "\t" << qsMod2 <<  endl;
}
QProcess::execute("xmgrace", QString("-param data_d.par -log y data_d.txt -hardcopy -hdevice JPEG -printfile %1-%2-%3d.jpg")
                  .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd-hh:mm:ss"))
                  .arg(iStrob)
                  .arg(iBeam)
                  .split(" "));
QFile qfData_v("data_v.txt");
if (!qfData_v.open(QIODevice::ReadWrite)) return 3;
QTextStream tsDataOut_v(&qfData_v);
qfData_v.resize(0);
for (int kDoppler=qMax(iDopplerFrom,NFFT/2); kDoppler<iDopplerTo; kDoppler++) {
    int kDoppler1=kDoppler-NFFT;
    int idx=2*(kDoppler*iFilteredN+iMax);
    tsDataOut_v << QString::number(kDoppler1*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
}
for (int kDoppler=iDopplerFrom; kDoppler<qMin(iDopplerTo,NFFT/2); kDoppler++) {
    int idx=2*(kDoppler*iFilteredN+iMax);
    tsDataOut_v << QString::number(kDoppler*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
}
QProcess::execute("xmgrace", QString("-batch data_v.bat -param data_v.par -log y data_v.txt -hardcopy -hdevice JPEG -printfile %1-%2-%3v.jpg")
                  .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd-hh:mm:ss"))
                  .arg(iStrob)
                  .arg(iBeam)
                  .split(" "));
#endif
//################################################################################################################
