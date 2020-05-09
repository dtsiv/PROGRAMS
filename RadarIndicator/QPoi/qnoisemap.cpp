#include "qnoisemap.h"
#include "qpoi.h"
#include "qexceptiondialog.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QNoiseMap::QNoiseMap(QString qsNoisefMapFName /* = "noisemap.dat" */ ) :
          m_qsNoisefMapFName(qsNoisefMapFName)
        , m_nStrobs(0)
        , m_nStrobsTotal(0)
        , m_nRecs(0)
        , m_dGlobal_dB(0.0e0) {
    m_qfNoiseMap.setFileName(m_qsNoisefMapFName);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int QNoiseMap::readNoiseMap(  // read noise map from binary data file
        unsigned int iFilteredN,
        unsigned int NT_,
        unsigned int Np,
        int NFFT) {
    int iRet=0;

    //===================== input file
    if (!m_qfNoiseMap.open(QIODevice::ReadOnly)) {
        qDebug() << "m_qfNoiseMap open failed" << endl;
        return 2;
    }
    struct noiseMapHeader imHeader;
    quint64 uBytesRead;
    uBytesRead = m_qfNoiseMap.read((char*)&imHeader,sizeof(struct noiseMapHeader));
    if (uBytesRead != sizeof(struct noiseMapHeader)) {
        qDebug() << "qfNoiseMap read failed" << endl;
        return 3;
    }
    if (  imHeader.vMaj != NOISE_MAP_VERSION_MAJOR
       || imHeader.vMin != NOISE_MAP_VERSION_MINOR
       || imHeader.Np   != Np
       || imHeader.NT_  != NT_
       || imHeader.iFilteredN != iFilteredN
       || imHeader.NFFT != NFFT) {
        qDebug() << "imHeader.vMaj != NOISE_MAP_VERSION_MAJOR: " << imHeader.vMaj << " " << NOISE_MAP_VERSION_MAJOR;
        qDebug() << "imHeader.vMin != NOISE_MAP_VERSION_MINOR: " << imHeader.vMin << " " << NOISE_MAP_VERSION_MINOR;
        qDebug() << "imHeader.Np != Np: " << imHeader.Np << " " << Np;
        qDebug() << "imHeader.NT_" << imHeader.NT_ << " " << NT_;
        qDebug() << "imHeader.iFilteredN" << imHeader.iFilteredN << " " << iFilteredN;
        qDebug() << "imHeader.NFFT" << imHeader.NFFT << " " << NFFT;
        return 4;
    }

    // Now that NFFT, iFilteredN are known - reserve place in memory

    double *pAvrRe=(double *)m_baAvrRe.data();
    double *pAvrIm=(double *)m_baAvrIm.data();
    double *pAvrM1=(double *)m_baAvrM1.data();
    double *pAvrM2=(double *)m_baAvrM2.data();
    double *pAvrM3=(double *)m_baAvrM3.data();
    double *pAvrM4=(double *)m_baAvrM4.data();

    double dAvrM2Global = 0.0e0;
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            struct noiseMapRecord imRecord;
            uBytesRead = m_qfNoiseMap.read((char*)&imRecord,sizeof(struct noiseMapRecord));
            if (uBytesRead != sizeof(struct noiseMapRecord)) {
                qDebug() << "struct noiseMapRecord read failed" << endl;
                return 5;
            }
            pAvrRe[kDoppler*iFilteredN+iDelay]=imRecord.iRe;
            pAvrIm[kDoppler*iFilteredN+iDelay]=imRecord.iIm;
            pAvrM1[kDoppler*iFilteredN+iDelay]=imRecord.uM1;
            pAvrM2[kDoppler*iFilteredN+iDelay]=imRecord.uM2;
            pAvrM3[kDoppler*iFilteredN+iDelay]=imRecord.uM3;
            pAvrM4[kDoppler*iFilteredN+iDelay]=imRecord.uM4;

            dAvrM2Global += pAvrM2[kDoppler*iFilteredN+iDelay];
        }
    }
    dAvrM2Global /= (iFilteredN * NFFT);
    m_dGlobal_dB = 10.0e0 * log(dAvrM2Global) / log(10.0e0);

    m_qfNoiseMap.close();
    return iRet;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QNoiseMap::resizeNoiseAvrArrays(unsigned int iFilteredN,int NFFT,bool bZeroFill /* = false */) {
    int iSize = NFFT*iFilteredN*sizeof(double);
    if (bZeroFill) { // resize and fill
        m_baAvrRe.fill('\0',iSize);
        m_baAvrIm.fill('\0',iSize);
        m_baAvrM1.fill('\0',iSize);
        m_baAvrM2.fill('\0',iSize);
        m_baAvrM3.fill('\0',iSize);
        m_baAvrM4.fill('\0',iSize);
        m_nStrobs=0;
        m_nRecs=0;
    }
    else { // just resize, fill later by averaging over strobs
        m_baAvrRe.resize(iSize);
        m_baAvrIm.resize(iSize);
        m_baAvrM1.resize(iSize);
        m_baAvrM2.resize(iSize);
        m_baAvrM3.resize(iSize);
        m_baAvrM4.resize(iSize);
    }
    m_nRecs=0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double QNoiseMap::getProgress() {
    return (double)m_nStrobs/m_nStrobsTotal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void writeNoiseMapFile(QString qsFileName);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoi::checkStrobeParams(
               int iNp,              /* pStructStrobeHeader->pCount */
               int iNtau,            /* pStructStrobeHeader->pDuration */
               int iNT,              /* pStructStrobeHeader->pPeriod */
               int iNT_,             /* pStructStrobeHeader->distance */
               int iBeamCountsNum    /* beamCountsNum = NT_ * Np */
               ) {
    if (iNp != this->Np) return false;
    if (iNtau != this->Ntau) return false;
    if (iNT != this->NT) return false;
    if (iNT_ != this->NT_) return false;
    // if (this->iFilteredN != this->NT_ - this->Ntau + 1)
    if (this->iFilteredN != this->NT_)
    if (iBeamCountsNum != this->iBeamCountsNum) return false;
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoi::appendStrobToNoiseMap(QByteArray baSamples[QPOI_NUMBER_OF_BEAMS]) {
    for (int iBeam = 0; iBeam < QPOI_NUMBER_OF_BEAMS; iBeam++) {
        if (baSamples[iBeam].size() != iBeamCountsNum*iSizeOfComplex) return false;
    }

    double *pAvrRe=(double *)m_pNoiseMap->m_baAvrRe.data();
    double *pAvrIm=(double *)m_pNoiseMap->m_baAvrIm.data();
    // double *pAvrM1=(double *)m_pNoiseMap->m_baAvrM1.data();
    double *pAvrM2=(double *)m_pNoiseMap->m_baAvrM2.data();
    // double *pAvrM3=(double *)m_pNoiseMap->m_baAvrM3.data();
    // double *pAvrM4=(double *)m_pNoiseMap->m_baAvrM4.data();

    // Sum over QPOI_NUMBER_OF_BEAMS beams as array of doubles
    QByteArray baSamplesDP;
    baSamplesDP.resize(iBeamCountsNum*2*sizeof(double));
    double *pDataDP = (double *)baSamplesDP.data();
    for (int i=0; i<2*iBeamCountsNum; i++) {
        pDataDP[i] = 0.0e0;
    }
    for (int iBeam = 0; iBeam < QPOI_NUMBER_OF_BEAMS; iBeam++) {
        qint16 *pData = (qint16 *)baSamples[iBeam].data();
        for (int i=0; i<2*iBeamCountsNum; i++) {
            pDataDP[i] += 1.0e0*pData[i];
        }
    }
    QByteArray baDopplerRepresentation = dopplerRepresentation(baSamplesDP);
    if (baDopplerRepresentation.isEmpty()) {
        qDebug() << "baDopplerRepresentation.isEmpty()";
        return false;
    }
    if (baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)) {
        qDebug() << "baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)";
        return false;
    }
    double *pDopplerData=(double*)baDopplerRepresentation.data();
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            double dRe,dIm,dM2;
            int idx1=kDoppler*iFilteredN+iDelay;
            int idx=2*idx1;
            dRe=pDopplerData[idx];
            dIm=pDopplerData[idx+1];
            dM2=dRe*dRe+dIm*dIm;
            // use pre-calculated noise map
            pAvrRe[idx1]+=dRe;
            pAvrIm[idx1]+=dIm;
            pAvrM2[idx1]+=dM2;
        }
    }
    m_pNoiseMap->m_nRecs++;
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoi::calcAverages() {
    if (!m_pNoiseMap->m_nRecs) return false;
    quint32 uSize=iFilteredN*NFFT*sizeof(double);
    if (m_pNoiseMap->m_baAvrRe.size() != uSize
     || m_pNoiseMap->m_baAvrIm.size() != uSize
     || m_pNoiseMap->m_baAvrM2.size() != uSize) {
        return false;
    }

    double *pAvrRe=(double *)m_pNoiseMap->m_baAvrRe.data();
    double *pAvrIm=(double *)m_pNoiseMap->m_baAvrIm.data();
    double *pAvrM1=(double *)m_pNoiseMap->m_baAvrM1.data();
    double *pAvrM2=(double *)m_pNoiseMap->m_baAvrM2.data();
    // double *pAvrM3=(double *)m_baAvrM3.data();
    // double *pAvrM4=(double *)m_baAvrM4.data();
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            int idx1=kDoppler*iFilteredN+iDelay;
            double dRe = pAvrRe[idx1];
            double dIm = pAvrIm[idx1];
            double dM2 = pAvrM2[idx1];
            int nRecs = m_pNoiseMap->m_nRecs;

            pAvrRe[idx1] = dRe = dRe/nRecs;
            pAvrIm[idx1] = dIm = dIm/nRecs;
            pAvrM2[idx1] = dM2/nRecs - dRe*dRe - dIm*dIm;
            if (pAvrM2[idx1] < 1.0e-8) {
                qDebug() << "QPoi::calcAverages(): pAvrM2[idx1] < 1.0e-8";
                return false;
            }
            pAvrM1[idx1] = sqrt(pAvrM2[idx1]);
        }
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoi::writeNoiseMapFile(QString qsFileName) {
    if (m_pNoiseMap->m_qfNoiseMap.isOpen()) {
        m_pNoiseMap->m_qfNoiseMap.close();
    }
    m_pNoiseMap->m_qfNoiseMap.setFileName(qsFileName);
    if (!m_pNoiseMap->m_qfNoiseMap.open(QIODevice::WriteOnly)) {
        throw RmoException("QPoi::writeNoiseMapFile(): open failed");
        return;
    }

    // file header
    QNoiseMap::noiseMapHeader sNoiseMapHeader;
    sNoiseMapHeader.vMaj = NOISE_MAP_VERSION_MAJOR;
    sNoiseMapHeader.vMin = NOISE_MAP_VERSION_MINOR;
    sNoiseMapHeader.Np   = Np;
    sNoiseMapHeader.NT_  = NT_;
    sNoiseMapHeader.iFilteredN = iFilteredN;
    sNoiseMapHeader.NFFT = NFFT;
    sNoiseMapHeader.uStrobsCount = m_pNoiseMap->m_nStrobs;
    quint64 uBytesWritten = m_pNoiseMap->m_qfNoiseMap.write((char*)&sNoiseMapHeader,sizeof(struct QNoiseMap::noiseMapHeader));
    if (uBytesWritten!=sizeof(struct QNoiseMap::noiseMapHeader)) {
        throw RmoException("QPoi::writeNoiseMapFile(): uBytesWritten!=sizeof(struct sNoiseMapHeader)");
        return;
    }

    double *pAvrRe=(double *)m_pNoiseMap->m_baAvrRe.data();
    double *pAvrIm=(double *)m_pNoiseMap->m_baAvrIm.data();
    double *pAvrM1=(double *)m_pNoiseMap->m_baAvrM1.data();
    double *pAvrM2=(double *)m_pNoiseMap->m_baAvrM2.data();
    double *pAvrM3=(double *)m_pNoiseMap->m_baAvrM3.data();
    double *pAvrM4=(double *)m_pNoiseMap->m_baAvrM4.data();
    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            int idx1=kDoppler*iFilteredN+iDelay;
            QNoiseMap::noiseMapRecord sNoiseMapRecord;
            sNoiseMapRecord.iRe=pAvrRe[idx1];
            sNoiseMapRecord.iIm=pAvrIm[idx1];
            sNoiseMapRecord.uM1=pAvrM1[idx1];
            sNoiseMapRecord.uM2=pAvrM2[idx1];
            sNoiseMapRecord.uM3=pAvrM3[idx1];
            sNoiseMapRecord.uM4=pAvrM4[idx1];
            uBytesWritten = m_pNoiseMap->m_qfNoiseMap.write((char*)&sNoiseMapRecord,sizeof(struct QNoiseMap::noiseMapRecord));
            if (uBytesWritten!=sizeof(struct QNoiseMap::noiseMapRecord)) {
                throw RmoException("QPoi::writeNoiseMapFile(): uBytesWritten!=sizeof(struct sNoiseMapRecord)");
                return;
            }
        }
    }
    m_pNoiseMap->m_qfNoiseMap.close();

    // output data file to plot noise spectrum with xmgrace
    if (m_bPlotNoiseSpec) {
        // loop bounds
        int iMaxDelayForPlot=30;
        int iMaxDopplerIdx=qMin(40,NFFT/2);

        // output file
        QFile qfSpectrumOutput(QDir::current().absolutePath()+"/noisespectrum.dat");
        qfSpectrumOutput.resize(0);
        if (!qfSpectrumOutput.open(QIODevice::WriteOnly)) {
            throw RmoException("QPoi::writeNoiseMapFile(): uBytesWritten!=sizeof(struct sNoiseMapRecord)");
            return;
        }
        QTextStream tsSpectrumResults(&qfSpectrumOutput);

        // double *pAvrRe=(double *)m_pNoiseMap->m_baAvrRe.data();
        // double *pAvrIm=(double *)m_pNoiseMap->m_baAvrIm.data();
        // double *pAvrM1=(double *)m_pNoiseMap->m_baAvrM1.data();
        double *pAvrM2=(double *)m_pNoiseMap->m_baAvrM2.data();
        // double *pAvrM3=(double *)m_pNoiseMap->m_baAvrM3.data();
        // double *pAvrM4=(double *)m_pNoiseMap->m_baAvrM4.data();

        // global avrM2
        double dAvrM2Global = 0.0e0;
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
                dAvrM2Global += pAvrM2[kDoppler*iFilteredN+iDelay];
            }
        }
        dAvrM2Global /= (iFilteredN * NFFT);
        double dGlobal_dB = 10.0e0 * log(dAvrM2Global) / log(10.0e0);

        //==================== plotting
        // phy scales
        double dVLight=299.79; // m/usec
        double dDoppFmin=1.0e0/(NT*m_dTs)/NFFT; // This is smallest Doppler frequency (MHz) possible for this sampling interval dTs
        double dVelCoef = 1.0e6*dVLight/2/m_dCarrierF; // Increment of target velocity (m/s) per 1 MHz of Doppler shift
        dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count (the "smallest" Doppler frequency possible)
        // double dDistCoef = m_dTs*dVLight/2; // m per sample (target distance increment per sampling interval dTs)

        for (int kDoppler=NFFT-iMaxDopplerIdx; kDoppler<NFFT; kDoppler++) {
            tsSpectrumResults << dVelCoef*(kDoppler - NFFT);
            for (int iDelay=0; iDelay<iMaxDelayForPlot; iDelay++) {
                double dPow = 10.0e0 * log(pAvrM2[kDoppler*iFilteredN+iDelay]) / log(10.0e0);
                tsSpectrumResults << "\t" << dPow - dGlobal_dB;
            }
            tsSpectrumResults << endl;
        }
        for (int kDoppler=0; kDoppler<iMaxDopplerIdx-1; kDoppler++) {
            tsSpectrumResults << dVelCoef*kDoppler;
            for (int iDelay=0; iDelay<iMaxDelayForPlot; iDelay++) {
                double dPow = 10.0e0 * log(pAvrM2[kDoppler*iFilteredN+iDelay]) / log(10.0e0);
                tsSpectrumResults << "\t" << dPow - dGlobal_dB;
            }
            tsSpectrumResults << endl;
        }
        qfSpectrumOutput.close();
    }
}
