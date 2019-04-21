#include <cmath>
#include "nr.h"
using namespace std;

#include "sqlmodel.h"
#include "poi.h"

QByteArray dopplerRepresentation1(qint16 *pData, unsigned int iArrElemCount,
                                 unsigned int NT, unsigned int NT_, unsigned int Ntau, unsigned int Np, int &NFFT);

//======================================================================================================
//
//======================================================================================================
int justDoppler() {
    int iRet=0;

    if ((iRet=openDatabase())) return iRet+10;

    bool bOk;
    QTextStream                  tsStdOut(stdout);
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT complexdata,seqnum,beam,ncnt,timestamp FROM"
                    " files f LEFT JOIN strobs s ON f.id=s.fileid"
                    " LEFT JOIN samples sa ON s.id=sa.strobid"
                    " WHERE seqnum BETWEEN :iRawStrobFrom AND :iRawStrobTo"
                    " AND beam=:beam"
                    " AND f.filepath LIKE :filepath");
    query.bindValue(":beam",iPlotSliceBeam);
    query.bindValue(":iRawStrobFrom",iRawStrobFrom);
    query.bindValue(":iRawStrobTo",iRawStrobTo);
    query.bindValue(":filepath",qsGetFileName());
    if (!query.exec()) return 1;
    QSqlRecord rec;
    rec = query.record();
    // tsStdOut << "size: " << query.size() << endl;
    // int iPlots=0;
    QFile qfDetectionResults("detection.txt");
    qfDetectionResults.open(QIODevice::ReadWrite);
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
        // if (QDateTime::fromMSecsSinceEpoch(iTimestamp) < QDateTime::fromString("2019-03-01T14-12-50","yyyy-MM-ddThh-mm-ss")) continue;

        int iArrElemCount = 2*Np*NT_;
        int iSizeOfComplex = 2*sizeof(qint16);
        if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

        int NFFT=0;

        QByteArray baDopplerRepresentation = dopplerRepresentation1(pData, iArrElemCount, NT, NT_, Ntau, Np, NFFT);
        if (NFFT!=2048) {
            tsStdOut << "NFFT!=2048" << endl;
            continue;
        }
        if (baDopplerRepresentation.isEmpty()) return 3;
        if (baDopplerRepresentation.size()<2*NT_*Np*sizeof(double)) return 4;
        double *pDopplerData=(double*)baDopplerRepresentation.data();

        iDelayFrom=qBound(0,iDelayFrom,NT_);
        iDelayTo=qBound(0,iDelayTo,NT_);
        iDopplerFrom=qBound(0,iDopplerFrom,NFFT);
        iDopplerTo=qBound(0,iDopplerTo,NFFT);

        QByteArray baSigMod2(baDopplerRepresentation.size(),0);
        double *pMod2=(double *)baSigMod2.data();
        double dMax2=0.0e0;
        int iMax=-1,kMax=-1;
        // search max mod2
        for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
            for (int kDoppler=iDopplerFrom; kDoppler<iDopplerTo; kDoppler++) {
                int idx=2*(kDoppler*NT_+iDelay);
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
            tsStdOut << "Error: kMax<0 || iMax<0: " << iStrob << "\t" << beamCountsNum << endl;
            continue;
        }
        if (dMax2<dThreshold) continue;

        // phy scales
        double dVLight=300.0; // m/usec
        double dDoppFmin=1.0e0/(NT*dTs)/NFFT; // MHz
        double dVelCoef = 1.0e6*dVLight/2/dCarrierF; // m/s per MHz
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count
        double dDistCoef = dTs*dVLight/2; // m per sample
        static int iOut=0;
        if (!iOut++) tsStdOut << "dDistCoef=" << dDistCoef << "\tdVelCoef=" << dVelCoef << endl;
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
        for (int iDelay=iDelayFrom; iDelay<iDelayTo; iDelay++) {
            int idx=2*(kMax*NT_+iDelay);
            double dRe=pDopplerData[idx];
            double dIm=pDopplerData[idx+1];
            QString qsMod2=QString::number(dRe*dRe+dIm*dIm);
            tsDataOut_d << QString::number(iDelay*dDistCoef) << "\t" << qsMod2 <<  endl;
        }
        QProcess::execute("gnuplot", QString(
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
        int iDopplerTo1 = qMin(dMaxVelocity/dVelCoef,NFFT/2.0e0);
        int iDopplerFrom1 = NFFT-iDopplerTo1;
        for (int kDoppler=iDopplerFrom1; kDoppler<iDopplerTo; kDoppler++) {
            int kDoppler1=kDoppler-NFFT;
            int idx=2*(kDoppler*NT_+iMax);
            tsDataOut_v << QString::number(kDoppler1*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
        }
        for (int kDoppler=iDopplerFrom; kDoppler<iDopplerTo1; kDoppler++) {
            int idx=2*(kDoppler*NT_+iMax);
            tsDataOut_v << QString::number(kDoppler*dVelCoef) << "\t" << QString::number(pMod2[idx]) << endl;
        }
        QProcess::execute("gnuplot", QString(
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
        if (1) { // dVelo > -100.0 && dVelo < -60.0) {
            tsDetectionResults
                << iStrob << "\t"
                << QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd") << "\t"
                << QString::number(dVelo,'f',1) << "\t"
                << QString::number(iMax*dDistCoef,'f',0) << "\t"
                << QString::number(dMax2,'g',3) << endl;
        }
    }
    return 0;
}

//======================================================================================================
//
//======================================================================================================
QByteArray dopplerRepresentation1(qint16 *pData, unsigned int iArrElemCount,
                                 unsigned int NT, unsigned int NT_, unsigned int Ntau, unsigned int Np, int &NFFT) {
    if (iArrElemCount!=2*NT_*Np || NT_<Ntau || NT<NT_) return QByteArray();
    unsigned int i,j,k;
    QTextStream tsStdOut(stdout);


    NFFT=0;
    for (i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    // if (NFFT!=2048) return QByteArray();

    QByteArray retVal(2*NFFT*NT_*sizeof(double),0);
    double *pRetData=(double *)retVal.data();
    Vec_DP inData(NFFT*2),vecZeroes(NFFT*2);
    int isign;
    for (i=0; i<NFFT*2; i++) vecZeroes[i]=0.0e0;

    // tsStdOut << "Fourier" << endl;
    // loop over echo delay
    for (i=0; i<NT_; i++) {
        inData=vecZeroes;
        // if (i==10) tsStdOut << i << "\t" << QString::number(inData[2*Np]) << "\t" << QString::number(inData[2*Np+1]) << endl;
        for (j=0; j<Np; j++) {
            int idxR=2*(j*NT_+i);
            inData[2*j]=pData[idxR];
            inData[2*j+1]=pData[idxR+1];
        }
        isign=1;
        NR::four1(inData,isign);
        for (k=0; k<NFFT; k++) {
            int idxW=2*(k*NT_+i);
            pRetData[idxW]=inData[2*k]/NFFT;
            pRetData[idxW+1]=inData[2*k+1]/NFFT;
        }
    }
    return retVal;
}
