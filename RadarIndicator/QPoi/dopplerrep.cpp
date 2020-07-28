#include "qpoi.h"
#include "qexceptiondialog.h"

#include "nr.h"
using namespace std;

//======================================================================================================
//
//======================================================================================================
QByteArray QPoi::dopplerRepresentation(QByteArray &baSamplesDP) {
    if (iBeamCountsNum != Np*NT_ || NT_<Ntau || NT<NT_
         // || iFilteredN!=NT_-Ntau+1
            || iFilteredN!=NT_) return QByteArray();
    int iDataSize = baSamplesDP.size();
    if (iDataSize!=iBeamCountsNum*2*sizeof(double)) {
        qDebug() << "iDataSize!=iBeamCountsNum*2*sizeof(double)";
        return QByteArray();
    }
    // pointer to array of complex pairs (double,double). This is INPUT array of size Np*NT_
    double *pData = (double*) baSamplesDP.data();

    int i,j,k;
    // loop over periods, in-place calculation
    // after matched filter the received signal array length is iFilteredN samples
    QByteArray baDataFiltered(2*Np*iFilteredN*sizeof(double),0);
    double *pDataFiltered=(double *)baDataFiltered.data();
    for (k=0; k<Np; k++) {
        // loop over one period (recorded fragment) 0...NT_-1
        for (i=0; i<iFilteredN; i++) { // index within one period
            qint32 iRe=0,iIm=0; // running cumulants
            int itau_min = qMax(0,Ntau-i-1);
            int itau_max = Ntau-1;
            for (int itau=itau_min; itau<Ntau; itau++) { // summation over pulse duration:
                                                         // itau=itau_min,itau_min+1,...,(Ntau-1)=itau_max
                int idx_in; // index of input array pData: idx_in=2*(k*NT_),...,2*(k*NT_+NT_-1)
                idx_in=2*(k*NT_ + (i-itau_max+itau) ); // k*NT_ - offset of period beginning;
                                                       // i - offset of filter output: i=0,1,...,(iFilteredN-1)
                                                       // (i-itau_max+itau) = max(0,i-Ntau+1), ... , i
                iRe+=pData[idx_in    ]; // the first array index is 2*(k*NT_)
                iIm+=pData[idx_in + 1]; // the last array index is 2*(k*NT_+NT_-1)+1 = 2*(k*NT_+NT_)-1
                // Thus within one period we span (2*NT_) pData array elements: 2*(k*NT_)+0, ..., 2*(k*NT_+NT_)-1
            }
            int idx; // index of output array pDataFiltered: idx=2*(k*NT_),...,2*(k*NT_+iFilteredN-1)
            idx=2*(k*iFilteredN+ i); // k*iFilteredN - offset of period beginning; i - offset of filter output: i=0,1,...,(iFilteredN-1)
            // Thus within one period we produce 2*(iFilteredN) pDataFiltered array elements of type double
            pDataFiltered[idx]=iRe; pDataFiltered[idx+1]=iIm;
        }
    }

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
bool QPoi::getPointDopplerRep(int iDelay, int kDoppler,
        QByteArray pbaBeamDataDP[QPOI_NUMBER_OF_BEAMS],
        double dBeamAmplRe[QPOI_NUMBER_OF_BEAMS],
        double dBeamAmplIm[QPOI_NUMBER_OF_BEAMS]) {
    // return true on success, false otherwise
    bool bRetVal = false;

    // Hamming window with size NFFT
    QByteArray baHammingWin(NFFT*sizeof(double),0);
    double *pHammingWin=(double *)baHammingWin.data();
    double dHamA0=25.0/46;
    double dHamA1=21.0/46;
    double dHamW=2.0*3.14159265/(Np-1);
    for (int j=0; j<Np; j++) {
        pHammingWin[j]=dHamA0-dHamA1*cos(dHamW*j);
    }
    for (int j=Np; j<NFFT; j++) {
        pHammingWin[j]=0.0e0;
    }

    // Complex amplitude for 4 beams at resolution element (iDelay, kDoppler)
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) {
        dBeamAmplRe[iBeam] = 0.0e0;
        dBeamAmplIm[iBeam] = 0.0e0;
    }

    // matched filter applied for all beams
    QByteArray baDataFiltered(2*Np*QPOI_NUMBER_OF_BEAMS*sizeof(double),(char)0);
    double *pDataFiltered = (double *)baDataFiltered.data();
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) { //  loop over beams
        double *pDataDP = (double *)pbaBeamDataDP[iBeam].data();
        for (int k=0; k<Np; k++) { // loop over periods
            // loop over one period (recorded fragment) 0...NT_-1
            int i = iDelay; // index within one period
            double dRe=0, dIm=0; // running cumulants
            int itau_min = qMax(0,Ntau-i-1);
            int itau_max = Ntau-1;
            for (int itau=itau_min; itau<Ntau; itau++) { // summation over pulse duration:
                                                         // itau=itau_min,itau_min+1,...,(Ntau-1)=itau_max
                int idx_in; // index of input array pData: idx_in=2*(k*NT_),...,2*(k*NT_+NT_-1)
                idx_in=2*(k*NT_ + (i-itau_max+itau) ); // k*NT_ - offset of period beginning;
                                                       // i - offset of filter output: i=0,1,...,(iFilteredN-1)
                                                       // (i-itau_max+itau) = max(0,i-Ntau+1), ... , i
                dRe+=pDataDP[idx_in    ]; // the first array index is 2*(k*NT_)
                dIm+=pDataDP[idx_in + 1]; // the last array index is 2*(k*NT_+NT_-1)+1 = 2*(k*NT_+NT_)-1
                // Thus within one period we span (2*NT_) pData array elements: 2*(k*NT_)+0, ..., 2*(k*NT_+NT_)-1
            }
            int idx; // index of output array pDataFiltered: idx=2*(k*4),...,2*(k*4+3)
            idx=2*(k*QPOI_NUMBER_OF_BEAMS + iBeam); //
            // Thus within one period we produce 2*(QPOI_NUMBER_OF_BEAMS) pDataFiltered array elements of type double
            pDataFiltered[idx]=dRe;
            pDataFiltered[idx+1]=dIm;
        }
    }

    // Doppler representation for all beams
    Vec_DP inData(NFFT*2),vecZeroes(NFFT*2);
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) { //  loop over beams
        inData=vecZeroes;
        for (int j=0; j<Np; j++) { // loop over periods
            int idx=2*(j*QPOI_NUMBER_OF_BEAMS+iBeam);
            inData[2*j]=pHammingWin[j]*pDataFiltered[idx];
            inData[2*j+1]=pHammingWin[j]*pDataFiltered[idx+1];
        }
        int isign=1;
        NR::four1(inData,isign);
        dBeamAmplRe[iBeam] = inData[2*kDoppler]/NFFT;
        dBeamAmplIm[iBeam] = inData[2*kDoppler+1]/NFFT;
    }
    bRetVal = true;
    return bRetVal;
}

//======================================================================================================
//
//======================================================================================================
bool QPoi::updateNFFT() {
    if (this->Np <= 0) return false;
    // number of points in FFT
    int iNFFT;
    for (int i=0; i<31; i++) {
        iNFFT = 1<<i;
        if (this->Np <= iNFFT) break;
        iNFFT=0;
    }
    if (iNFFT < this->Np || iNFFT > 2*this->Np) return false;
    this->NFFT = iNFFT;
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QPoi::updateStrobeParams(int iNp,              /* pStructStrobeHeader->pCount */
                              int iNtau,            /* pStructStrobeHeader->pDuration */
                              int iNT,              /* pStructStrobeHeader->pPeriod */
                              int iNT_,             /* pStructStrobeHeader->distance */
                              int iBeamCountsNum    /* beamCountsNum = NT_ * Np */
                              ) {
    if (iBeamCountsNum != iNT_ * iNp) return false;
    this->Np   = iNp;        // количество импульсов в стробе (число периодов = 1024)
    this->Ntau = iNtau;      // Длительность импульса (число выборок в одном импульсе = 8)
    this->NT   = iNT;        // Полное число выборок в одном периоде (повторения импульсов = 200)
    this->NT_  = iNT_;       // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
    // this->iFilteredN = (this->NT_ - this->Ntau + 1); // число выборок после согласованного фильра
    this->iFilteredN = this->NT_; // число выборок после согласованного фильра
    this->iBeamCountsNum = iBeamCountsNum;
    if (m_bUseLog) {
        QFile qfSimuLog(QDir::current().absolutePath()+"/simulog.txt");
        qfSimuLog.resize(0);
        if (!qfSimuLog.open(QIODevice::WriteOnly)) {
            QExceptionDialog *pDlg = new QExceptionDialog("QPoi::updateStrobeParams(): Failed to open log "+qfSimuLog.fileName());
            pDlg -> setAttribute(Qt::WA_DeleteOnClose);
            pDlg -> open();
            return true;
        }
        QTextStream tsSimuLog(&qfSimuLog);
        tsSimuLog << QString("Sounding parameters during simulation ") << QDateTime::currentDateTime().toString("dddd, d MMMM yy hh:mm:ss") << endl;
        tsSimuLog << QString("количество импульсов в стробе (число периодов): ") << this->Np << endl;
        tsSimuLog << QString("Длительность импульса (число выборок в одном импульсе): ") << iNtau << endl;
        tsSimuLog << QString("Полное число выборок в одном периоде (повторения импульсов): ") << iNT << endl;
        tsSimuLog << QString("Дистанция приема (число выборок, регистрируемых в одном периоде): ") << iNT_ << endl;
        tsSimuLog << QString("число выборок после согласованного фильра: ") << this->iFilteredN << endl;
        tsSimuLog << QString("Полное регистрируемое число выборок в одном стробе: ") << this->iBeamCountsNum << endl;
        tsSimuLog << QString("Время выборки (мкс): ") << this->m_dTs << endl;
        tsSimuLog << QString("Несущая частота (МГц): ") << this->m_dCarrierF << endl;

        // close simulation file
        qfSimuLog.close();
    }
    return true;
}
