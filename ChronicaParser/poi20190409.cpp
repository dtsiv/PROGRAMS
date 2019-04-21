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

//======================================================================================================
//
//======================================================================================================
int poi20190409() {
    int iRet=0;
    QTextStream tsStdOut(stdout);
    static int iCnt=0;

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
    qfDetectionResults.resize(0);
    if (!qfDetectionResults.open(QIODevice::ReadWrite)) tsStdOut << "open failed" << endl;
    // qfDetectionResults.seek(qfDetectionResults.size());
    QTextStream tsDetectionResults(&qfDetectionResults);
    int iStrobDetect=0;
    int iTotStrobs=0;

    double *pN2samples=NULL;
    int nN2samplesMax=24*1.0e6;
    int iN2samplesTot=0;
    try {
        pN2samples = new double[nN2samplesMax];
    } catch(std::exception& e) {
        tsStdOut << QString("Exception thrown: ") + e.what() << endl;
        return 1;
    }

#if 0
    {
        const int NP=100;
        string txt;
        int i;
        Vec_DP a(NP);
        ifstream fp("tarray.dat");

        if (fp.fail())
          NR::nrerror("Data file tarray.dat not found");
        getline(fp,txt);
        for (i=0;i<NP;i++) fp >> a[i];
        fp.close();
        cout << endl << "original array:" << endl;
        cout << fixed << setprecision(2);
        print_array(a,10,7);
        NR::sort(a);
        cout << endl << "sorted array:" << endl;
        print_array(a,10,7);
        return 0;
    }
#endif







    // qint64 iListSize=0;
    qint64 iListCnt=0;
    int iListCntMod=1013;
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
        int iUID = dtTimeStampRef.secsTo(dtTimeStamp)*100.0e0+ iStrob;
        iTotStrobs++;
//        if (iUID!=5109720) continue;

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

        QByteArray baMod2(baDopplerRepresentation.size(),0);
        double *pMod2=(double *)baMod2.data();
        // Mod2
        for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
            for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
                int idx=2*(kDoppler*iFilteredN+iDelay);
                double dRe=pDopplerData[idx];
                double dIm=pDopplerData[idx+1];
                pMod2[idx]=dRe*dRe+dIm*dIm;
            }
        }
        QByteArray baN2M(baDopplerRepresentation.size(),0);
        double *pN2M=(double *)baN2M.data();
        // noise estimation
        for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
            for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
                double dN2Mavr=0.0e0;
#if 1
                for (int iShift=-7; iShift<=7; iShift++) {
                    if ( iShift==-1 || iShift==0 || iShift==1) continue;
                    int k1=kDoppler+iShift;
                    if (k1<0) k1+=NFFT;
                    if (k1>=NFFT) k1-=NFFT;
                    int idx=2*(k1*iFilteredN+iDelay);
                    dN2Mavr+=pMod2[idx];
                }
                int idx=2*(kDoppler*iFilteredN+iDelay);
                pN2M[idx]=dN2Mavr/12;
#endif
#if 0
                for (int iShift=-16; iShift<=16; iShift+=8) {
                    if (iShift==0) continue;
                    int i1=iDelay+iShift;
                    if (i1<0) i1+=iFilteredN;
                    if (i1>=iFilteredN) i1-=iFilteredN;
                    if (i1<4) i1=4;
                    int idx=2*(kDoppler*iFilteredN+i1);
                    dN2Mavr+=pMod2[idx];
                }
                int idx=2*(kDoppler*iFilteredN+iDelay);
                pN2M[idx]=dN2Mavr/4;
#endif
            }
        }

        int *pKC=new int[iFilteredN*NFFT];
        int *pLC=new int[iFilteredN*NFFT];
        double *pY2MC=new double[iFilteredN*NFFT];
        int nc=0;
        for (int iDelay=3; iDelay<iFilteredN; iDelay++) {
            for (int kDoppler=2; kDoppler<NFFT-1; kDoppler++) {
#if 0
                if (kDoppler==1639 || kDoppler==409
                 || kDoppler==1640 || kDoppler==408
                 || kDoppler==1604 || kDoppler==444
                 || kDoppler==1680 || kDoppler==368
                 || kDoppler==1550 || kDoppler==498
                 || kDoppler==1588 || kDoppler==460
                 || kDoppler==1745 || kDoppler==303
                 || kDoppler==1725 || kDoppler==323
                 || kDoppler==1255 || kDoppler==793
                 || kDoppler==1310 || kDoppler==738
                 || kDoppler==1574 || kDoppler==474) continue;
#endif
                int idx=2*(kDoppler*iFilteredN+iDelay);
                if (!(iListCnt++%iListCntMod) && iN2samplesTot<nN2samplesMax && pN2M[idx]>1.0e-2) {
                    pN2samples[iN2samplesTot]=pMod2[idx]/pN2M[idx];
                    iN2samplesTot++;
                }
                // iListSize++;
                if (pMod2[idx]>dRelThresh*pN2M[idx]) { //  && pMod2[idx]<15*pN2M[idx]) {
                    pKC[nc]=kDoppler;
                    pLC[nc]=iDelay;
                    pY2MC[nc]=pMod2[idx];
                    // detection text file
#if 0
                    tsDetectionResults
                        << QString::number(iUID,'f',0)
                        << QString("\t%1\t%2").arg(kDoppler,4).arg(iDelay,2)
                        << "\t" << QString::number(pMod2[idx],'f',1)
                        << "\t" << QString::number(pN2M[idx],'f',1)
                        << endl;
#endif
#if 0
                    tsDetectionResults
                        << QString::number(iUID,'f',0)
                        << QString("\t%1\t%2").arg(kDoppler,4).arg(iDelay,2)
                        << "\t" << QString::number(pMod2[idx],'f',1)
                        << "\t" << QString::number(pN2M[idx],'f',1);
                    tsDetectionResults << "\t:";
                    for (int iShift=-16; iShift<=16; iShift++) {
                        int i1=iDelay+iShift;
                        if (i1<0) i1+=iFilteredN;
                        if (i1>=iFilteredN) i1-=iFilteredN;
                        int idx=2*(kDoppler*iFilteredN+i1);
                        if (iShift==-8)tsDetectionResults << "\t[";
                        tsDetectionResults << "\t" << pMod2[idx];
                        if (iShift==8)tsDetectionResults << "\t]";
                    }
                    tsDetectionResults << endl;
#endif
                    // tsStdOut << kDoppler << "\t" << iDelay << "\t" << pMod2[idx] << endl;
                    nc++;
                }
            }
        }
            //<< QDateTime::fromMSecsSinceEpoch(iTimestamp).toString("dd") << "\t"
            //<< QString::number(dVelo,'f',1) << "\t"
            //<< QString::number(iMax*dDistCoef,'f',0) << "\t"
            //<< QString::number(dMax2,'g',3) << endl;
#if 0
        if (nc>0) {
            for (int ic=0; ic<nc; ic++) {
                // bool bMirror=false;
                // for (int ic1=0; ic1<nc; ic1++) {
                //     if (ic1==ic) continue;
                //     if (pKC[ic1]==pKC[ic]) bMirror=true;
                // }
                // if (bMirror) continue;
                tsDetectionResults
                    << QString::number(iUID,'f',0)
                    << QString("\t%1\t%2").arg(pKC[ic],4).arg(pLC[ic],2)
                    << "\t" << QString::number(pY2MC[ic],'f',1)
                    << endl;
            }
        }
#endif
        delete[] pKC;
        delete[] pLC;
        delete[] pY2MC;
#if 0
        if (nc) {
            tsDetectionResults << iUID << "\t" << nc << "\t" << iStrob << endl;
        }
#endif

        if (nc) iStrobDetect++;
    }
    tsStdOut << "List size:\t" << iN2samplesTot << endl;
    // start sort
    Vec_DP *pN2vec=NULL;
    try {
        pN2vec = new Vec_DP(iN2samplesTot);
    } catch(std::exception& e) {
        tsStdOut << QString("new Vec_DP: Exception thrown: ") + e.what() << endl;
        return 1;
    }
    // copy for sort
    for (int i=0; i<iN2samplesTot; i++) {
        (*pN2vec)[i] = pN2samples[i];
    }
    NR::sort(*pN2vec);
    double dDistrMax=(*pN2vec)[iN2samplesTot-1];
    int iBin,nBinHits,iIdxCurr=0,nDistrBins=100;
    double dBinSize=dDistrMax/nDistrBins,dBinUpper;
    for (iBin=0; iBin<nDistrBins; iBin++) {
        dBinUpper=dBinSize*(iBin+1);
        nBinHits=0;
        while (iIdxCurr<iN2samplesTot && ((*pN2vec)[iIdxCurr] < dBinUpper)) {
            iIdxCurr++;
            nBinHits++;
        }
        if (iIdxCurr>=iN2samplesTot) break;
        tsDetectionResults << QString("%1\t%2").arg(dBinUpper).arg(1.0e0*nBinHits/iN2samplesTot/dBinSize/exp(-dBinUpper)) << endl;
    }
    // clean memory
    if (pN2samples) delete pN2samples;
    if (pN2vec) delete pN2vec;
    return 0;
}
