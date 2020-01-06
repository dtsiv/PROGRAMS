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

#include "qchrprotoacm.h"

#include "qavtctrl.h"
#include "codograms.h"

#include "winbase.h"

int idum=-13;

#define SKIP_ZERO_DOPPLER          5
#define SKIP_ZERO_DELAY            3
#define NOISE_AVERAGING_N          7

// #define USE_GNUPLOT
#define USE_TEXT_DEBUG             1

int iFilteredN;
int iNumberOfBeams=4;
QByteArray baYM2forBeams;
QByteArray baDetectionForBeams;
QByteArray baDopplerForBeams;
QByteArray baDelayForBeams;
QFile qfTbl("latextbl.txt");
QTextStream tsTbl(&qfTbl);
QFile qfPowTbl("pawertbl.txt");
QTextStream tsPowTbl(&qfPowTbl);

//============================================================================================
//
//============================================================================================
void outputLatexPowerLine(int iStrob) {
    // phy scales
    double dVLight=300.0; // m/usec
    int NFFT=1024;
    double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // This is smallest Doppler frequency (MHz) possible for this sampling interval dTs
    double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // Increment of target velocity (m/s) per 1 MHz of Doppler shift
    dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count (the "smallest" Doppler frequency possible)
    double dDistCoef = dTs*dVLight/2; // m per sample (target distance increment per sampling interval dTs)

    //===
    int baYM2forBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baYM2forBeams.size()!=baYM2forBeamsSize) {
        qDebug() << "clearPowerList(): baYM2forBeams.size()!=baYM2forBeamsSize";
        return;
    }
    double *pYM2forBeams = (double *)baYM2forBeams.data();
    //===
    int baDetectionForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(bool);
    if (baDetectionForBeams.size()!=baDetectionForBeamsSize) {
        qDebug() << "clearPowerList(): baDetectionForBeams.size()!=baDetectionForBeamsSize";
        return;
    }
    bool *pDetectionForBeams = (bool *)baDetectionForBeams.data();
    //===
    int baDelayForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baDelayForBeams.size()!=baDelayForBeamsSize) {
        qDebug() << "clearPowerList(): baDelayForBeams.size()!=baDelayForBeamsSize";
        return;
    }
    double *pDelayforBeams = (double *)baDelayForBeams.data();
    //===
    int baDopplerForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baDopplerForBeams.size()!=baDopplerForBeamsSize) {
        qDebug() << "clearPowerList(): baDopplerForBeams.size()!=baDopplerForBeamsSize";
    }
    double *pDopplerforBeams = (double *)baDopplerForBeams.data();

    double dDopplerMain=0;
    double dDelayMain=0;
    int kDopplerMain=0;
    int iDelayMain=0;
    bool bDetection=false;
    double dY2max=0;
    for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
        for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
            for (int iBeam=0; iBeam<iNumberOfBeams; iBeam++) {
                if (!pDetectionForBeams[iBeam*NFFT*iFilteredN+kDoppler*iFilteredN+iDelay]) continue;
                bDetection=true;
                double dY2 = pYM2forBeams[iBeam*NFFT*iFilteredN+kDoppler*iFilteredN+iDelay];
                if (dY2max < dY2) {
                    dY2max = dY2;
                    dDopplerMain=pDopplerforBeams[iBeam*NFFT*iFilteredN+kDoppler*iFilteredN+iDelay];
                    dDelayMain=pDelayforBeams[iBeam*NFFT*iFilteredN+kDoppler*iFilteredN+iDelay];
                    kDopplerMain=kDoppler;
                    iDelayMain=iDelay;
                }
            }
        }
    }
    if (!bDetection) return;
    double dVel;
    if (dDopplerMain >= NFFT/2) {
        dVel=dVelCoef*(dDopplerMain-NFFT);
    }
    else {
        dVel=dVelCoef*dDopplerMain;
    }
    tsPowTbl << iStrob
             << " & " << QString("%1").arg(dDelayMain*dDistCoef,0,'f',1)
             << " & " << QString("%1").arg(dVel,0,'f',1);
    for (int iBeam=0; iBeam<iNumberOfBeams; iBeam++) {
        double dY2 = pYM2forBeams[iBeam*NFFT*iFilteredN+kDopplerMain*iFilteredN+iDelayMain];
        tsPowTbl << " & " << QString("%1").arg(dY2,0,'f',1);
    }
    tsPowTbl << " \\\\ " << endl;
}
//============================================================================================
//
//============================================================================================
void clearPowerList() {
    int NFFT=1024;
    //===
    int baYM2forBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baYM2forBeams.size()!=baYM2forBeamsSize) {
        qDebug() << "clearPowerList(): baYM2forBeams.size()!=baYM2forBeamsSize";
        return;
    }
    double *pYM2forBeams = (double *)baYM2forBeams.data();
    //===
    int baDetectionForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(bool);
    if (baDetectionForBeams.size()!=baDetectionForBeamsSize) {
        qDebug() << "clearPowerList(): baDetectionForBeams.size()!=baDetectionForBeamsSize";
        return;
    }
    bool *pDetectionForBeams = (bool *)baDetectionForBeams.data();
    //===
    int baDelayForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baDelayForBeams.size()!=baDelayForBeamsSize) {
        qDebug() << "clearPowerList(): baDelayForBeams.size()!=baDelayForBeamsSize";
        return;
    }
    double *pDelayforBeams = (double *)baDelayForBeams.data();
    //===
    int baDopplerForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
    if (baDopplerForBeams.size()!=baDopplerForBeamsSize) {
        qDebug() << "clearPowerList(): baDopplerForBeams.size()!=baDopplerForBeamsSize";
    }
    double *pDopplerforBeams = (double *)baDopplerForBeams.data();

    for (int iBeam=0; iBeam<iNumberOfBeams; iBeam++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
                pDetectionForBeams[iBeam*NFFT*iFilteredN + kDoppler*iFilteredN+iDelay] = false;
                pYM2forBeams[iBeam*NFFT*iFilteredN + kDoppler*iFilteredN+iDelay] = 0;
                pDelayforBeams[iBeam*NFFT*iFilteredN + kDoppler*iFilteredN+iDelay]=0;
                pDopplerforBeams[iBeam*NFFT*iFilteredN + kDoppler*iFilteredN+iDelay]=0;
            }
        }
    }
}

//======================================================================================================
//
//======================================================================================================
int poi20191231() {
    int iRet=0;
    QTextStream tsStdOut(stdout);

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    // double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    QSqlRecord rec;
    if (!query.exec("SELECT filever FROM files LIMIT 1")) return 1;
    rec = query.record();
    if (!query.next()) return 1;
    quint32 uFileVersion = query.value(rec.indexOf("filever")).toUInt(&bOk);
    if (!bOk) return 1;

//    bOk=query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
//                   " files f LEFT JOIN strobs s ON f.id=s.fileid"
//                   " LEFT JOIN samples sa ON s.id=sa.strobid"
//                   " WHERE beam=:beam AND f.filepath LIKE :filepath"
//                   );
    bOk=query.prepare("SELECT complexdata,seqnum,beam,ncnt,structStrobData FROM"
                    " strobs s LEFT JOIN files f ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " ORDER BY s.seqnum ASC"
                    );
//    query.bindValue(":beam",iPlotSliceBeam);
//    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    rec = query.record();
    QFile qfDetectionResults("detection.dat");
    qfDetectionResults.resize(0);
    if (!qfDetectionResults.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    QTextStream tsDetectionResults(&qfDetectionResults);

    // Latex table
    qfTbl.resize(0);
    if (!qfTbl.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    qfPowTbl.resize(0);
    if (!qfPowTbl.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;

    QByteArray baStructStrobData;

    int iStrobsDetect=0;
    while (query.next()) {
        int iStrob=query.value(rec.indexOf("seqnum")).toInt(&bOk);
        int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
        int iBeam=query.value(rec.indexOf("beam")).toInt(&bOk);
        // if (iBeam != iPlotSliceBeam) return 2;
        baStructStrobData=query.value(rec.indexOf("structStrobData")).toByteArray();
        QByteArray baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
        qint16 *pData = (qint16*) baSamples.data();
        char* pStructStrobData = (char*)baStructStrobData.data();
        QString qsExecTime("n/a");
        QString qsBeta("n/a");
        QString qsEpsilon("n/a");
        if (uFileVersion==REG::FORMAT_VERSION) {
            static qulonglong uStartTime=(qulonglong)(-1);
            ACM::STROBE_DATA *pStrobeData=(ACM::STROBE_DATA *)pStructStrobData;
            //FILETIME ftExecTime=pStrobeData->header.execTime;
            //SYSTEMTIME st;
            //FileTimeToSystemTime(&ftTlock,&st);
            //qsExecTime = QString("%1-%2-%3T%4:%5:%6.%7")
            //              .arg(st.wYear)
            //              .arg(st.wMonth,2,10,QLatin1Char('0'))
            //              .arg(st.wDay,2,10,QLatin1Char('0'))
            //              .arg(st.wHour,2,10,QLatin1Char('0'))
            //              .arg(st.wMinute,2,10,QLatin1Char('0'))
            //              .arg(st.wSecond,2,10,QLatin1Char('0'))
            //              .arg(st.wMilliseconds,3,10,QLatin1Char('0'));
            if (uStartTime==(qulonglong)(-1)) {
                QFile qfTime("timeunits.txt");
                qfTime.resize(0);
                qfTime.open(QIODevice::ReadWrite);
                QTextStream tsTime(&qfTime);
                uStartTime = pStrobeData->header.execTime;
                tsTime << "uStartTime = " << uStartTime << endl;
                tsTime << "StartStrobe = " << iStrob << endl;
                qfTime.close();
            }
            qsExecTime = QString::number((pStrobeData->header.execTime-uStartTime)*1.0e-7);
            qsBeta = QString::number(pStrobeData->beamPos.beamBeta);
            qsEpsilon = QString::number(pStrobeData->beamPos.beamEpsilon);
            if (iBeam == 0) {
                tsStdOut << QString::number((pStrobeData->header.execTime-uStartTime)*1.0e-7)
                         << "\t" << QString::number(iBeam)
                         << "\t" << QString::number(pStrobeData->beamPos.beamEpsilon)
                         << "\t" << QString::number(iStrob)
                         << endl;
            }
        }
        int iDataSize = baSamples.size();

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        iFilteredN=NT_-Ntau+1; // in-place array size
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

        // power table output
        static bool bInit=false;
        int baYM2forBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
        if (baYM2forBeams.size()<baYM2forBeamsSize) {baYM2forBeams.resize(baYM2forBeamsSize); }
        double *pYM2forBeams = (double *)baYM2forBeams.data();
        int baDetectionForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(bool);
        if (baDetectionForBeams.size()<baDetectionForBeamsSize) {baDetectionForBeams.resize(baDetectionForBeamsSize);}
        bool *pDetectionForBeams = (bool *)baDetectionForBeams.data();
        int baDopplerForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
        if (baDopplerForBeams.size()<baDopplerForBeamsSize) {baDopplerForBeams.resize(baDopplerForBeamsSize); }
        double *pDopplerforBeams = (double *)baDopplerForBeams.data();
        int baDelayForBeamsSize = iFilteredN*NFFT*iNumberOfBeams*sizeof(double);
        if (baDelayForBeams.size()<baDelayForBeamsSize) {baDelayForBeams.resize(baDelayForBeamsSize); }
        double *pDelayforBeams = (double *)baDelayForBeams.data();
        if (!bInit) {
            bInit=true;
            clearPowerList();
        }

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
                // record power for all beams
                pYM2forBeams[iBeam*NFFT*iFilteredN+(kDoppler*iFilteredN+iDelay)]=pY2M[idx];
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
                double dAvrRe=pAvrRe[kDoppler*iFilteredN+iDelay];
                double dAvrIm=pAvrIm[kDoppler*iFilteredN+iDelay];
                double dAvrM2=pAvrM2[kDoppler*iFilteredN+iDelay];
                double dSqrtM2=sqrt(dAvrM2);
                Vec_DP dSignal(2);
                dSignal[0]=(pDopplerData[idx]-dAvrRe)/dSqrtM2;
                dSignal[1]=(pDopplerData[idx+1]-dAvrIm)/dSqrtM2;
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
                    double dAvrRe=pAvrRe[kDoppler*iFilteredN+iDelay];
                    double dAvrIm=pAvrIm[kDoppler*iFilteredN+iDelay];
                    double dAvrM2=pAvrM2[kDoppler*iFilteredN+iDelay];
                    double dSqrtM2=sqrt(dAvrM2);
                    dNoise[idxNoise++]=(pDopplerData[idx]-dAvrRe)/dSqrtM2;
                    dNoise[idxNoise++]=(pDopplerData[idx+1]-dAvrIm)/dSqrtM2;
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
            if (iBeam==3) {
                outputLatexPowerLine(iStrob);
                clearPowerList();
            }
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
                // output to file of detections
                if (dVel>0) {
                    int iDelayInt    =lmtg.at(itg);
                    int kDopplerInt  =kmtg.at(itg);
                    pDetectionForBeams[iBeam*NFFT*iFilteredN + kDopplerInt*iFilteredN+iDelayInt]=true;
                    // center of mass for signal
                    pDelayforBeams[iBeam*NFFT*iFilteredN + kDopplerInt*iFilteredN+iDelayInt]=iDelay;
                    pDopplerforBeams[iBeam*NFFT*iFilteredN + kDopplerInt*iFilteredN+iDelayInt]=kDoppler;
                    tsDetectionResults << QString("%1\t%2\t%3\t%4\t%5\t%6\t%7")
                                          .arg(qsExecTime)
                                          .arg(iDelay*dDistCoef)
                                          .arg(dVel)
                                          .arg(iBeam)
                                          .arg(qsEpsilon)
                                          .arg(iStrob)
                                          .arg(dY2M)
                                      << endl;

                    ACM::STROBE_DATA *pStrobeData=(ACM::STROBE_DATA *)baStructStrobData.data();
                    tsTbl << pStrobeData->header.execTime
                          << " & " << iStrob
                          << " & " << iBeam
                          << " & " << pStrobeData->beamPos.beamEpsilon
                          << " & " << QString("%1").arg(iDelay*dDistCoef,0,'f',1)
                          << " & " << QString("%1").arg(dVel,0,'f',1)
                          << " \\\\ " << endl;
                }
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
        baStructStrobData.clear();

        if (iBeam==3) {
            outputLatexPowerLine(iStrob);
            clearPowerList();
        }
    }
    // tsStdOut << "Number of strobs with candidates:\t" << iStrobsDetect << endl;
    return 0;
}

void clearPowerLine() {

}
