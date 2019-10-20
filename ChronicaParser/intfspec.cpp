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

#define INTF_MAP_VERSION_MAJOR 1
#define INTF_MAP_VERSION_MINOR 0

struct intfMapHeader {
    quint32 vMaj;
    quint32 vMin;
    quint32 Np;
    quint32 NT_;
    quint32 iFilteredN;
    quint32 NFFT;
    quint32 uStrobsCount;
};

struct intfMapRecord {
    double iRe;   // average real component of signal in Doppler rep.
    double iIm;   // average imag component
    double uM1;  // average signal modulus
    double uM2;  // average signal modulus2
    double uM3;  // average signal modulus3
    double uM4;  // average signal modulus4
};

//======================================================================================================
//
//======================================================================================================
int interferenceSpectrum() {
    int iRet=0;
    QTextStream tsStdOut(stdout);

    //==================== arrays
    // Fourier size
    int NFFT=0;
    for (int i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    if (NFFT!=1024 && NFFT!=2048) return 2;
    // distance
    int iFilteredN=NT_-Ntau+1; // in-place array size

    //===================== output file
    QFile qfSpectrumOutput("intfspectrum.txt");
    qfSpectrumOutput.resize(0);
    if (!qfSpectrumOutput.open(QIODevice::ReadWrite)) {
        tsStdOut << "open failed" << endl;
        return 1;
    }
    QTextStream tsSpectrumResults(&qfSpectrumOutput);

    //===================== input file
    QFile qfIntfMap("intfmap.dat");
    if (!qfIntfMap.open(QIODevice::ReadOnly)) {
        tsStdOut << "qfIntfMap open failed" << endl;
        return 2;
    }
    struct intfMapHeader imHeader;
    quint64 uBytesRead;
    uBytesRead = qfIntfMap.read((char*)&imHeader,sizeof(struct intfMapHeader));
    if (uBytesRead != sizeof(struct intfMapHeader)) {
        tsStdOut << "qfIntfMap read failed" << endl;
        return 3;
    }
    if (  imHeader.vMaj != INTF_MAP_VERSION_MAJOR
       || imHeader.vMin != INTF_MAP_VERSION_MINOR
       || imHeader.Np   != Np
       || imHeader.NT_  != NT_
       || imHeader.iFilteredN != iFilteredN
       || imHeader.NFFT != NFFT) {
        tsStdOut << "qfIntfMap header failed" << endl;
        return 4;
    }


    QByteArray baAvrRe(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrRe=(double *)baAvrRe.data();
    QByteArray baAvrIm(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrIm=(double *)baAvrIm.data();
    QByteArray baAvrM1(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM1=(double *)baAvrM1.data();
    QByteArray baAvrM2(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM2=(double *)baAvrM2.data();
    QByteArray baAvrM3(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM3=(double *)baAvrM3.data();
    QByteArray baAvrM4(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM4=(double *)baAvrM4.data();

    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            struct intfMapRecord imRecord;
            uBytesRead = qfIntfMap.read((char*)&imRecord,sizeof(struct intfMapRecord));
            if (uBytesRead != sizeof(struct intfMapRecord)) {
                tsStdOut << "struct intfMapRecord read failed" << endl;
                return 5;
            }
            pAvrRe[kDoppler*iFilteredN+iDelay]=imRecord.iRe;
            pAvrIm[kDoppler*iFilteredN+iDelay]=imRecord.iIm;
            pAvrM1[kDoppler*iFilteredN+iDelay]=imRecord.uM1;
            pAvrM2[kDoppler*iFilteredN+iDelay]=imRecord.uM2;
            pAvrM3[kDoppler*iFilteredN+iDelay]=imRecord.uM3;
            pAvrM4[kDoppler*iFilteredN+iDelay]=imRecord.uM4;
        }
    }

#if 0
    //==================== plotting
    for (int kDoppler=NFFT/2; kDoppler<NFFT; kDoppler++) {
        tsSpectrumResults << kDoppler-NFFT;
        for (int iDelay=0; iDelay<30; iDelay++) {
            //double dDiffSqr = (double)pAvrM2[kDoppler*iFilteredN+iDelay];
            //dDiffSqr = (double)pAvrM4[kDoppler*iFilteredN+iDelay] - dDiffSqr*dDiffSqr;
            //dDiffSqr = sqrt(dDiffSqr);
            //tsSpectrumResults << "\t" << dDiffSqr / (double)pAvrM2[kDoppler*iFilteredN+iDelay];
            tsSpectrumResults << "\t" << (double)pAvrM2[kDoppler*iFilteredN+iDelay];
        }
        tsSpectrumResults << endl;
    }
    for (int kDoppler=0; kDoppler<NFFT/2; kDoppler++) {
        tsSpectrumResults << kDoppler;
        for (int iDelay=0; iDelay<30; iDelay++) {
            //double dDiffSqr = (double)pAvrM2[kDoppler*iFilteredN+iDelay];
            //dDiffSqr = (double)pAvrM4[kDoppler*iFilteredN+iDelay] - dDiffSqr*dDiffSqr;
            //dDiffSqr = sqrt(dDiffSqr);
            //tsSpectrumResults << "\t" << dDiffSqr / (double)pAvrM2[kDoppler*iFilteredN+iDelay];
            tsSpectrumResults << "\t" << (double)pAvrM2[kDoppler*iFilteredN+iDelay];
        }
        tsSpectrumResults << endl;
    }
#endif

    for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
        tsSpectrumResults << kDoppler;
        for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
            double iRe=pAvrRe[kDoppler*iFilteredN+iDelay];
            double iIm=pAvrIm[kDoppler*iFilteredN+iDelay];
            double dM2=pAvrM2[kDoppler*iFilteredN+iDelay];
            tsSpectrumResults
              << "\t" << dM2-iRe*iRe-iIm*iIm
          //  << "\t" << pAvrRe[kDoppler*iFilteredN+iDelay]
          //  << "\t" << pAvrIm[kDoppler*iFilteredN+iDelay]
          //  << "\t" << pAvrM1[kDoppler*iFilteredN+iDelay]
          //  << "\t" << pAvrM2[kDoppler*iFilteredN+iDelay]
          //  << "\t" << pAvrM3[kDoppler*iFilteredN+iDelay]
          //  << "\t" << pAvrM4[kDoppler*iFilteredN+iDelay]
            ;
        }
        tsSpectrumResults << endl;
    }

    return iRet;
}
//======================================================================================================
//
//======================================================================================================
int interferenceMap() {
    int iRet=0;
    QTextStream tsStdOut(stdout);

    if ((iRet=openDatabase())) return iRet+10;
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    // number of strobs for averaging
    bOk=query.prepare("select count(*) as totstrobs from files f "
                      " left join strobs s on f.id=s.fileid "
                      " left join samples sa on sa.strobid=s.id "
                      " where (select count(*) from strobs where fileid=f.id)>1;"
                    );
    if (!query.exec()) return 1;
    if (query.next()) {
        QSqlRecord recTotStrobs = query.record();
        quint64 uTotStrobs=query.value(recTotStrobs.indexOf("totstrobs")).toULongLong(&bOk);
        if (bOk) {
            tsStdOut << "Total strobs: " << uTotStrobs << endl;
        }
        else return 1;
    }
    else {
        return 1;
    }

    // Fourier size
    int NFFT=0;
    for (int i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    if (NFFT!=1024 && NFFT!=2048) return 2;

    // distance
    int iFilteredN=NT_-Ntau+1; // in-place array size

    // total data per beam
    int iArrElemCount = 2*Np*NT_;

    // intf map header
    QFile qfIntfMap("intfmap.dat");
    qfIntfMap.resize(0);
    if (!qfIntfMap.open(QIODevice::ReadWrite)) {
        tsStdOut << "qfIntfMap open failed" << endl;
        return 2;
    }
    struct intfMapHeader imHeader;
    imHeader.vMaj = INTF_MAP_VERSION_MAJOR;
    imHeader.vMin = INTF_MAP_VERSION_MINOR;
    imHeader.Np = Np;
    imHeader.NT_= NT_;
    imHeader.iFilteredN = iFilteredN;
    imHeader.NFFT = NFFT;
    quint64 uBytesWritten;
    uBytesWritten = qfIntfMap.write((char*)&imHeader,sizeof(struct intfMapHeader));
    if (uBytesWritten!=sizeof(struct intfMapHeader)) {
        tsStdOut << "uBytesWritten!=sizeof(struct intfMapHeader)" << endl;
        return 2;
    }



    // sample estimates
    quint64 uStrobsTot=0;
    QByteArray baAvrRe(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrRe=(double *)baAvrRe.data();
    QByteArray baAvrIm(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrIm=(double *)baAvrIm.data();
    QByteArray baAvrM1(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM1=(double *)baAvrM1.data();
    QByteArray baAvrM2(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM2=(double *)baAvrM2.data();
    QByteArray baAvrM3(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM3=(double *)baAvrM3.data();
    QByteArray baAvrM4(NFFT*iFilteredN*sizeof(double),0);
    double *pAvrM4=(double *)baAvrM4.data();
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            pAvrRe[kDoppler*iFilteredN+iDelay]=0;
            pAvrIm[kDoppler*iFilteredN+iDelay]=0;
            pAvrM1[kDoppler*iFilteredN+iDelay]=0;
            pAvrM2[kDoppler*iFilteredN+iDelay]=0;
            pAvrM3[kDoppler*iFilteredN+iDelay]=0;
            pAvrM4[kDoppler*iFilteredN+iDelay]=0;
        }
    }

    // list individual files
    QSqlQuery queryFiles(db);
    bOk=queryFiles.prepare("SELECT f.id as fileid"
                    " FROM files f"
                    " ORDER BY f.timestamp,f.filepath ASC");
    if (!bOk || !queryFiles.exec()) return 1;
    QSqlRecord recFiles = queryFiles.record();
    quint64 uFileId=0;
    while (queryFiles.next()) {
        uFileId=queryFiles.value(recFiles.indexOf("fileid")).toInt(&bOk);
        if (!bOk) {
            tsStdOut << "uFileId....toInt(&bOk)==false" << endl;
            return 1;
        }
        tsStdOut << "uFileId = " << uFileId << endl;

        bOk=query.prepare("SELECT "
                        " complexdata"
                        " ,seqnum"
                        " ,beam"
                        " ,ncnt"
                        " ,timestamp"
                        " FROM"
                        " files f LEFT JOIN strobs s ON f.id=s.fileid"
                        " LEFT JOIN samples sa ON s.id=sa.strobid"
                        " WHERE f.id=:fileid"
                        " AND (select count(*) from strobs where fileid=f.id)>1"
                        " ORDER BY s.seqnum ASC"
                        );
        query.bindValue(":fileid",uFileId);
        if (!bOk || !query.exec()) {
            tsStdOut << "query.prepare=" << bOk << " !bOk || !query.exec() == true" << endl;
            return 1;
        }
        QSqlRecord rec = query.record();
        // calculate map of averages M2
        QByteArray baDopplerRepresentation;
        QByteArray baSamples;
        while (query.next()) {
            baSamples=query.value(rec.indexOf("complexdata")).toByteArray();
            qint16 *pData = (qint16*) baSamples.data();
            int beamCountsNum=query.value(rec.indexOf("ncnt")).toInt(&bOk);
            int iDataSize = baSamples.size();
            int iSizeOfComplex = 2*sizeof(qint16);
            if (beamCountsNum != Np*NT_ || iDataSize!=Np*NT_*iSizeOfComplex) return 2;

            // to doppler representation
            baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount, iFilteredN, NT, NT_, Ntau, Np, NFFT);
            if (NFFT!=1024 && NFFT!=2048) {
                tsStdOut << "NFFT!=1024 && NFFT!=2048" << endl;
                continue;
            }
            baSamples.clear();
            if (baDopplerRepresentation.isEmpty()) return 3;
            if (baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)) return 4;
            double *pDopplerData=(double*)baDopplerRepresentation.data();
            for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
                for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
                    double dRe,dIm;
                    int idx=2*(kDoppler*iFilteredN+iDelay);
                    dRe=pDopplerData[idx];
                    dIm=pDopplerData[idx+1];
                    pAvrRe[kDoppler*iFilteredN+iDelay]+=dRe;
                    pAvrIm[kDoppler*iFilteredN+iDelay]+=dIm;
                    double dM2=dRe*dRe+dIm*dIm;
                    double dM1=sqrt(dM2);
                    pAvrM1[kDoppler*iFilteredN+iDelay]+=sqrt(dM2);
                    pAvrM2[kDoppler*iFilteredN+iDelay]+=dM2;
                    double dM3=dM1*dM2;
                    pAvrM3[kDoppler*iFilteredN+iDelay]+=dM3;
                    double dM4=dM2*dM2;
                    pAvrM4[kDoppler*iFilteredN+iDelay]+=dM4;
                }
            }
            baDopplerRepresentation.clear();
            ++uStrobsTot;
            if ((uStrobsTot % 1000) == 0) tsStdOut << uStrobsTot << endl;
            // if (uStrobsTot > 20000) break;
        } // loop over strobs in one file
    } // loop over files
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            struct intfMapRecord imRecord;
            imRecord.iRe=pAvrRe[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            imRecord.iIm=pAvrIm[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            imRecord.uM1=pAvrM1[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            imRecord.uM2=pAvrM2[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            imRecord.uM3=pAvrM3[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            imRecord.uM4=pAvrM4[kDoppler*iFilteredN+iDelay]/uStrobsTot;
            quint64 uBytesWritten;
            uBytesWritten = qfIntfMap.write((char*)&imRecord,sizeof(struct intfMapRecord));
            if (uBytesWritten!=sizeof(struct intfMapRecord)) {
                tsStdOut << "uBytesWritten!=sizeof(struct intfMapRecord)" << endl;
                return 5;
            }
        }
    }
    tsStdOut << "Processed: " << uStrobsTot << endl;
    qfIntfMap.close();
    return iRet;
}

