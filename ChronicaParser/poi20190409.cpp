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

#define SKIP_ZERO_DOPPLER          50
#define SKIP_ZERO_DELAY            3
#define NOISE_AVERAGING_N          7

void avevar_poi(Vec_I_DP &data, DP &ave, DP &var)
{
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

void ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob)
{
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
int poi20190409() {
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
                    " WHERE beam=:beam AND f.filepath LIKE :filepath"
                    );
    query.bindValue(":beam",iPlotSliceBeam);
    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    QFile qfDetectionResults("detection.txt");
    qfDetectionResults.resize(0);
    if (!qfDetectionResults.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    QTextStream tsDetectionResults(&qfDetectionResults);

    int iTotStrobs=0;
    int iStrobsDetect=0;
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        qint64 iTimestamp=query.value(rec.indexOf("timestamp")).toLongLong(&bOk);
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        int iDataSize = baSamples.size();
        QDateTime dtTimeStamp=QDateTime::fromMSecsSinceEpoch(iTimestamp);
        QDateTime dtTimeStampRef=QDateTime::fromString("2019-03-01T00-00-00","yyyy-MM-ddThh-mm-ss");

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        int iFilteredN=NT_-Ntau+1; // in-place array size
        int NFFT=0;


// if (iStrob!=202) continue;

        QByteArray baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount, iFilteredN, NT, NT_, Ntau, Np, NFFT);
        if (NFFT!=2048) {
            tsStdOut << "NFFT!=2048" << endl;
            continue;
        }
        if (baDopplerRepresentation.isEmpty()) return 3;
        if (baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)) return 4;
        double *pDopplerData=(double*)baDopplerRepresentation.data();
        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // MHz
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // m/s per MHz
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count
        double dDistCoef = dTs*dVLight/2; // m per sample

        // qfDetectionResults.resize(0);
        int nc=0;
        tsStdOut << iTotStrobs++ << endl;

        double dAvrM2=0.0e0;
        int iAvrM2=0;
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
                pY2M[idx]=dRe*dRe+dIm*dIm;
                dAvrM2+=pY2M[idx];
                iAvrM2++;
            }
        }
        dAvrM2/=iAvrM2;

        // candidates detection
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
                if (pY2M[idx]>dAvrM2*dThreshold) continue;
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
                if (dFrac > dRelThresh && dProb < dFalseAlarmProb) {
                    nc++;
                    kc.append(kDoppler);
                    lc.append(iDelay);
                    y2mc.append(pY2M[idx]); 

                    // tsDetectionResults << QString("%1\t%2\t%3\t%4\t%5\t%6")
                    //                        .arg(iDelay*dDistCoef,15) 
                    //                        .arg(dVel,15)
                    //                        .arg(dProb,15)
                    //                        .arg(dFrac,15)
                    //                        .arg((dRe*dRe+dIm*dIm)/dAvrM2,15)
                    //                        .arg(iStrob,15)
                    //                    << endl;
                    
                }
            }
        }
        if (!nc) continue;

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
            if (tgs.at(itg)>1) {
                tsDetectionResults << QString("%1\t%2\t%3\t%4\t%5")
                                       .arg(iDelay*dDistCoef,15) 
                                       .arg(dVel,15)
                                       .arg(dY2M/dAvrM2,15)
                                       .arg(iStrob,15)
                                       .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyMMdd-hh:mm"))
                                   << endl;
                bTargetsListed=true;
            }
        }

        if (0 && bTargetsListed) {
            QProcess::execute("gnuplot", QString(
                                  "-e_""set terminal png"""
                                  "_-e_""set title '%1-%2' font 'arial,18'"""
                                  "_-e_""set output '%1-%2-v.png'"""
                                  "_-e_""set xrange [%3:%4]"""
                                  "_-e_""set yrange [%5:%6]"""
                                  "_-e_""set xtics 100"""
                                  "_-e_""set ytics 100"""
                                  "_-e_""set grid"""
                                  "_-e_""plot 'detection.txt' with points pointtype 2 pointsize 2 linewidth 2 notitle""" )
                              .arg(QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("yyyy-MM-dd-hh:mm:ss"))
                              .arg(iStrob)
                              .arg(SKIP_ZERO_DELAY*dDistCoef)
                              .arg(iFilteredN*dDistCoef)
                              .arg(-(NFFT/2-SKIP_ZERO_DOPPLER)*dVelCoef)
                              .arg( (NFFT/2-SKIP_ZERO_DOPPLER)*dVelCoef)
                              .split("_"));
            iStrobsDetect++;
        }
    }
    tsStdOut << "Number of strobs with candidates:\t" << iStrobsDetect << endl;
    return 0;
}

