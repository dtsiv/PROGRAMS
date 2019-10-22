#include <QtGlobal>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "nr.h"
using namespace std;

#include "sqlmodel.h"
#include "poi.h"

int idum=-13;

#define SKIP_ZERO_DOPPLER          5
#define SKIP_ZERO_DELAY            3
#define NOISE_AVERAGING_N          7

// #define USE_GNUPLOT
#define USE_TEXT_DEBUG             1

//======================================================================================================
//
//======================================================================================================
int poi20191016() {
    int iRet=0;
    QTextStream tsStdOut(stdout);

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    // double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
//    bOk=query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
//                   " files f LEFT JOIN strobs s ON f.id=s.fileid"
//                   " LEFT JOIN samples sa ON s.id=sa.strobid"
//                   " WHERE beam=:beam AND f.filepath LIKE :filepath"
//                   );
    bOk=query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " strobs s LEFT JOIN files f ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " ORDER BY s.seqnum ASC"
                    );
//    query.bindValue(":beam",iPlotSliceBeam);
//    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    QFile qfDetectionResults("detection.txt");
    qfDetectionResults.resize(0);
    if (!qfDetectionResults.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    QTextStream tsDetectionResults(&qfDetectionResults);

    int iStrobsDetect=0;
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        // if (iBeam != iPlotSliceBeam) return 2;
        qint64 iTimestamp=query.value(rec.indexOf("timestamp")).toLongLong(&bOk);
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        int iDataSize = baSamples.size();

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        int iFilteredN=NT_-Ntau+1; // in-place array size
        int NFFT=0;

        // Doppler representation
        QByteArray baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount, iFilteredN, NT, NT_, Ntau, Np, NFFT);
        if (NFFT!=1024 && NFFT!=2048) {
            tsStdOut << "NFFT!=1024 && NFFT!=2048" << endl;
            continue;
        }
        if (baDopplerRepresentation.isEmpty()) return 3;
        if (baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)) return 4;
        double *pDopplerData=(double*)baDopplerRepresentation.data();
        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // This is smallest Doppler frequency (MHz) possible for this sampling interval dTs
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // Increment of target velocity (m/s) per 1 MHz of Doppler shift
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count (the "smallest" Doppler frequency possible)
        double dDistCoef = dTs*dVLight/2; // m per sample (target distance increment per sampling interval dTs)

        // Interference map
        QByteArray baAvrRe(NFFT*iFilteredN*sizeof(double),0);
        QByteArray baAvrIm(NFFT*iFilteredN*sizeof(double),0);
        QByteArray baAvrM1(NFFT*iFilteredN*sizeof(double),0);
        QByteArray baAvrM2(NFFT*iFilteredN*sizeof(double),0);
        QByteArray baAvrM3(NFFT*iFilteredN*sizeof(double),0);
        QByteArray baAvrM4(NFFT*iFilteredN*sizeof(double),0);
        double *pAvrRe=(double *)baAvrRe.data();
        double *pAvrIm=(double *)baAvrIm.data();
        // double *pAvrM1=(double *)baAvrM1.data();
        double *pAvrM2=(double *)baAvrM2.data();
        // double *pAvrM3=(double *)baAvrM3.data();
        // double *pAvrM4=(double *)baAvrM4.data();
        iRet = readInterferenceMap(iFilteredN, NT_, Np, NFFT,
                baAvrRe, baAvrIm, baAvrM1, baAvrM2, baAvrM3, baAvrM4);
        if (iRet) {
            tsStdOut << "readInterferenceMap failed: " << iRet << endl;
            return 10+iRet;
        }

        //================================================================================================================================
        //  ok, at this point the Doppler representation (Re,Im)=(pDopplerData[2*(k*iFilteredN+i)],pDopplerData[2*(k*iFilteredN+i)+1])
        //  where k = 0,...,(NFFT-1) is Doppler frequency index, currently 20191016 this is 0,...,1023
        //  and   i = 0, ..., (iFilteredN-1) is delay index, currently 20191016 this is 0,...,72
        //================================================================================================================================

        // initialize number of candidates nc
        int nc=0;

        // calculate globalmodulus squared of signal
        double dAvrM2global=0.0e0;
        int iAvrM2global=0;
        if (NFFT <= 2*SKIP_ZERO_DOPPLER) qFatal("negative NFFT-SKIP_ZERO_DOPPLER");
        QByteArray baY2M(baDopplerRepresentation.size(),0);
        double *pY2M=(double*)baY2M.data();
        // average Modulus2
        for (int iDelay=SKIP_ZERO_DELAY; iDelay<iFilteredN; iDelay++) {
            for (int kDoppler=SKIP_ZERO_DOPPLER; kDoppler<NFFT-SKIP_ZERO_DOPPLER; kDoppler++) {
                double dRe,dIm;
                int idx=2*(kDoppler*iFilteredN+iDelay);
                dRe=pDopplerData[idx];
                dIm=pDopplerData[idx+1];
                // use pre-calculated interference map
                double dAvrRe=pAvrRe[kDoppler*iFilteredN+iDelay];
                double dAvrIm=pAvrIm[kDoppler*iFilteredN+iDelay];
                double dAvrM2=pAvrM2[kDoppler*iFilteredN+iDelay];
                // subtract average
                dRe-=dAvrRe;
                dIm-=dAvrIm;
                // calculate Interference dispersion
                dAvrM2-=dAvrRe*dAvrRe;
                dAvrM2-=dAvrIm*dAvrIm;
                if (dAvrM2 < 0.0e0) {
                    tsStdOut << "dAvrM2 < 0.0e0"  << endl;
                    return 5;
                }
                // calculate signal in Doppler representation
                pY2M[idx]=dRe*dRe+dIm*dIm;
                // rescale with respect to interference
                pY2M[idx]/=dAvrM2;
                // Auxiliary global average
                dAvrM2global+=pY2M[idx];
                iAvrM2global++;
            }
        }
        dAvrM2global/=iAvrM2global;

        // candidates detection
        double dFracMax=0.0e0, dProbMin=1.0e0;
        QList<int> kc,lc;
        QList<double> y2mc;
        for (int iDelay=SKIP_ZERO_DELAY; iDelay<iFilteredN; iDelay++) {
            for (int kDoppler1=0; kDoppler1<NFFT-2*SKIP_ZERO_DOPPLER; kDoppler1++) {
                int kDoppler = NFFT/2+kDoppler1;
                if (kDoppler >= NFFT-SKIP_ZERO_DOPPLER) kDoppler=kDoppler-NFFT+2*SKIP_ZERO_DOPPLER;
                int idx=2*(kDoppler*iFilteredN+iDelay);
                Vec_DP dSignal(2);
                dSignal[0]=pDopplerData[idx];
                dSignal[1]=pDopplerData[idx+1];
                // if (pY2M[idx]>dAvrM2global*dThreshold) continue;
                Vec_DP dNoise((2*NOISE_AVERAGING_N-2)*2);
                int idxNoise=0;
                for (int iShift=-NOISE_AVERAGING_N; iShift<=NOISE_AVERAGING_N; iShift++) {
                    if (iShift==-1 || iShift==0 || iShift==1) continue;
                    int k1=kDoppler+iShift;
                    if (k1<0) k1+=NFFT;
                    if (k1>=NFFT) k1-=NFFT;
                    int idx=2*(k1*iFilteredN+iDelay);
                    if (idxNoise>dNoise.size()-2) qFatal("Array size error");
                    dNoise[idxNoise++]=pDopplerData[idx];
                    dNoise[idxNoise++]=pDopplerData[idx+1];
                }
                DP dFrac,dProb;
                ftest_poi(dNoise,dSignal,dFrac,dProb);
                dFracMax=(dFracMax>dFrac)?dFracMax:dFrac;
                dProbMin=(dProbMin<dProb)?dProbMin:dProb;

                if (dFrac > dThreshold && dProb < dFalseAlarmProb) {
                    nc++;
                    kc.append(kDoppler);
                    lc.append(iDelay);
                    y2mc.append(pY2M[idx]); 

                    if (USE_TEXT_DEBUG > 1) {
                        double dVel;
                        if (kDoppler >= NFFT/2) {
                            dVel=dVelCoef*(kDoppler-NFFT);
                        }
                        else {
                            dVel=dVelCoef*kDoppler;
                        }
                        tsStdOut << iStrob << "\t" << iBeam << "\t" << iDelay*dDistCoef << "\t" << dVel << "\t" << log(pY2M[idx]) << endl;
                    }
                }
            }
        }
        if (!nc) {
            if(USE_TEXT_DEBUG > 1) {
                tsStdOut << iStrob << "\t" << dFracMax << "\t" << dProbMin << endl;
            }
            continue;
        }

        // multiple targets resolution
        int ntg=0;
        QList<int> tgs;
        QList<double> ktg,ltg;
        QList<double> y2mtg;
        QList<int> lmtg,kmtg;
        int tglmax=8;

        for (int ic=0; ic<nc; ic++) {
            bool bFlag=false;
            for (int itg=0; itg<ntg; itg++) {
                int ktg_=ktg.at(itg);
                int ltg_=ltg.at(itg);
                int kc_=kc.at(ic);
                int lc_=lc.at(ic);
                // test delay difference
                if (abs(lc_-ltg_)>tglmax) continue;
                // test Doppler channel difference
                double kdiff=kc_-ktg_;
                if (kdiff>NFFT/2) kdiff-=NFFT;
                if (kdiff<-NFFT/2) kdiff+=NFFT;
                if (abs(kdiff)>1) continue;
                // candidate associated with itg
                bFlag=true;
                int tgs_=tgs.at(itg)+1;
                // recalculate k-center
                kc_=ktg_+kdiff;
                ktg[itg]=(ktg.at(itg)*tgs.at(itg)+kc_)/tgs_;
                if (ktg.at(itg)>NFFT) ktg[itg]-=NFFT;
                if (ktg.at(itg)<0) ktg[itg]+=NFFT;
                // recalculate l-center
                ltg[itg]=(ltg.at(itg)*tgs.at(itg)+lc_)/tgs_;
                tgs[itg]=tgs_;
                // update target maximum M2
                if (y2mtg.at(itg)<y2mc.at(ic)) {
                    y2mtg[itg]=y2mc.at(ic); 
                    lmtg[itg]=lc.at(ic);
                    kmtg[itg]=kc.at(ic);
                }
            }
            if (bFlag) continue;
            ltg.append(lc.at(ic));
            ktg.append(kc.at(ic));
            tgs.append(1);
            y2mtg.append(y2mc.at(ic));
            lmtg.append(lc.at(ic));
            kmtg.append(kc.at(ic));
            ntg=ntg+1;
        }

        bool bTargetsListed=false;
        for (int itg=0; itg<ntg; itg++) {
            double kDoppler=ktg.at(itg); 
            double iDelay=ltg.at(itg); 
            double dY2M=y2mtg.at(itg);
            double dVel;
            if (kDoppler >= NFFT/2) {
                dVel=dVelCoef*(kDoppler-NFFT);
            }
            else {
                dVel=dVelCoef*kDoppler;
            }
            if (tgs.at(itg)>5) {
                bTargetsListed=true;
                #ifdef USE_GNUPLOT
                // text-file output of detected target
                tsDetectionResults << QString("%1\t%2\t%3\t%4\t%5")
                                   .arg(iDelay*dDistCoef,15)
                                   .arg(dVel,15)
                                   .arg(dY2M/dAvrM2global,15)
                                   .arg(iStrob,15)
                                   .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyMMdd-hh:mm"))
                               << endl;

                #endif
                if (USE_TEXT_DEBUG) {
                    tsStdOut << QString("%1\t%2\t%3\t%4\t%5")
                                       .arg(iDelay*dDistCoef,15)
                                       .arg(dVel,15)
                                       .arg(dY2M/dAvrM2global,15)
                                       .arg(iStrob,15)
                                       .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyMMdd-hh:mm"))
                                   << endl;
                }
            }
        }

        if (bTargetsListed) {
            #ifdef USE_GNUPLOT
            QProcess::execute("gnuplot", QString(
                                  "-e_""set terminal png"""
                                  "_-e_""set title '%1-%2-%3' font 'arial,18'"""
                                  "_-e_""set output '%1-%2-%3-v.png'"""
                                  "_-e_""set xrange [%4:%5]"""
                                  "_-e_""set yrange [%6:%7]"""
                                  "_-e_""set xtics 100"""
                                  "_-e_""set ytics 5"""
                                  "_-e_""set grid"""
                                  "_-e_""plot 'detection.txt' with points pointtype 2 pointsize 2 linewidth 2 notitle""" )
                              .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                              .arg(iStrob)
                              .arg(iBeam)
                              .arg(SKIP_ZERO_DELAY*dDistCoef)
                              .arg(iFilteredN*dDistCoef)
                              .arg(-40*dVelCoef)
                              .arg( 40*dVelCoef)
//                              .arg(-(NFFT/2-SKIP_ZERO_DOPPLER)*dVelCoef)
//                              .arg( (NFFT/2-SKIP_ZERO_DOPPLER)*dVelCoef)
                              .split("_"));
            #endif
            iStrobsDetect++;

        }
        qfDetectionResults.resize(0);
    }
    tsStdOut << "Number of strobs with candidates:\t" << iStrobsDetect << endl;
    return 0;
}

