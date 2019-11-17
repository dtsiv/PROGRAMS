#include "qpoi.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include "nr.h"
using namespace std;

#define INTF_MAP_VERSION_MAJOR 1
#define INTF_MAP_VERSION_MINOR 0

#define SKIP_ZERO_DOPPLER          5
#define SKIP_ZERO_DELAY            3
#define NOISE_AVERAGING_N          7

int QPoi::m_idum = -13;

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QPoi::QPoi(QObject *parent)
          : QObject(parent)
          , m_dCarrierF(9500.0e0)
          , m_dTs(120.0e0)
          , m_pIntfMap(NULL) {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QPOI_CARRIER_FREQUENCY,9500.0e0);
    m_dCarrierF = iniSettings.value(QPOI_CARRIER_FREQUENCY,scRes).toDouble();
    iniSettings.setDefault(QPOI_SAMPLING_TIME_USEC,0.120e0);
    m_dTs = iniSettings.value(QPOI_SAMPLING_TIME_USEC,scRes).toDouble();
    iniSettings.setDefault(QPOI_INTFMAP,"intfmap.dat");
    QString qsIntfMapFName = iniSettings.value(QPOI_INTFMAP,scRes).toString();
    m_pIntfMap = new QIntfMap(qsIntfMapFName);
    iniSettings.setDefault(QPOI_NP,1024);
    Np = iniSettings.value(QPOI_NP,scRes).toInt();
    iniSettings.setDefault(QPOI_NTAU,8);
    Ntau = iniSettings.value(QPOI_NTAU,scRes).toInt();
    iniSettings.setDefault(QPOI_NT,200);
    NT = iniSettings.value(QPOI_NT,scRes).toInt();
    iniSettings.setDefault(QPOI_NT_,80);
    NT_ = iniSettings.value(QPOI_NT_,scRes).toInt();
    iniSettings.setDefault(QPOI_THRESHOLD,30.0e0);
    m_dThreshold = iniSettings.value(QPOI_THRESHOLD,scRes).toDouble();
    iniSettings.setDefault(QPOI_PFALARM,1.0e-8);
    m_dFalseAlarmProb = iniSettings.value(QPOI_PFALARM,scRes).toDouble();
    // number of samples after matched filter
    iFilteredN = NT_-Ntau+1;
    // number of points in FFT
    NFFT=0;
    for (int i=0; i<31; i++) {
        NFFT = 1<<i;
        if (Np<=NFFT) break;
        NFFT=0;
    }
    if (NFFT<Np || NFFT>2*Np) qDebug() << "Bad NFFT: " << NFFT;
    // read interference map from file
    int iRet=m_pIntfMap->readInterferenceMap(iFilteredN, NT_, Np, NFFT);
    if (iRet) {
        qDebug() << "readInterferenceMap() returned: " << iRet;
    }

    resetRandomNumberGenerators();
}
//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QPoi::~QPoi() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(QPOI_CARRIER_FREQUENCY, m_dCarrierF);
    iniSettings.setValue(QPOI_SAMPLING_TIME_USEC, m_dTs);
    if (m_pIntfMap) iniSettings.setValue(QPOI_INTFMAP, m_pIntfMap->m_qsIntfMapFName);
    iniSettings.setValue(QPOI_NP, Np);
    iniSettings.setValue(QPOI_NTAU, Ntau);
    iniSettings.setValue(QPOI_NT, NT);
    iniSettings.setValue(QPOI_NT_, NT_);
    iniSettings.setValue(QPOI_THRESHOLD, m_dThreshold);
    iniSettings.setValue(QPOI_PFALARM, m_dFalseAlarmProb);
    if (m_pIntfMap) delete m_pIntfMap;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoi::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QGridLayout *pGridLayout=new QGridLayout;
    pGridLayout->addWidget(new QLabel("Carrier frequency (MHz)"),0,0);
    pPropPages->m_pleFCarrier = new QLineEdit(QString::number(m_dCarrierF,'f',0));;
    pGridLayout->addWidget(pPropPages->m_pleFCarrier,0,1);

    pGridLayout->addWidget(new QLabel("Sampling time (microsec)"),1,0);
    pPropPages->m_pleTSampl = new QLineEdit(QString::number(m_dTs,'f',3));;
    pGridLayout->addWidget(pPropPages->m_pleTSampl,1,1);

    pGridLayout->setColumnStretch(2,100);
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pGridLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QPOI_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoi::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    bool bOk;
    QString qs;

    qs = pPropPages->m_pleFCarrier->text();
    double dCarrierF = qs.toDouble(&bOk);
    if (bOk) m_dCarrierF = qBound(5000.0e0,dCarrierF,15000.0e0);

    qs = pPropPages->m_pleTSampl->text();
    double dTs = qs.toDouble(&bOk);
    if (bOk) m_dTs = qBound(0.01e0,dTs,10.0e0); // microseconds
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QIntfMap::QIntfMap(QString qsIntfMapFName /* = "intfmap.dat" */ )
        : QObject(0)
        , m_qsIntfMapFName(qsIntfMapFName) {
    m_qfIntfMap.setFileName(m_qsIntfMapFName);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int QIntfMap::readInterferenceMap(  // read interference map from binary data file
        unsigned int iFilteredN,
        unsigned int NT_,
        unsigned int Np,
        int NFFT) {
    int iRet=0;

    //===================== input file
    if (!m_qfIntfMap.open(QIODevice::ReadOnly)) {
        qDebug() << "m_qfIntfMap open failed" << endl;
        return 2;
    }
    struct intfMapHeader imHeader;
    quint64 uBytesRead;
    uBytesRead = m_qfIntfMap.read((char*)&imHeader,sizeof(struct intfMapHeader));
    if (uBytesRead != sizeof(struct intfMapHeader)) {
        qDebug() << "qfIntfMap read failed" << endl;
        return 3;
    }
    if (  imHeader.vMaj != INTF_MAP_VERSION_MAJOR
       || imHeader.vMin != INTF_MAP_VERSION_MINOR
       || imHeader.Np   != Np
       || imHeader.NT_  != NT_
       || imHeader.iFilteredN != iFilteredN
       || imHeader.NFFT != NFFT) {
        qDebug() << "qfIntfMap header failed" << endl;
        return 4;
    }

    // Now that NFFT, iFilteredN are known - reserve place in memory
    m_baAvrRe.resize(NFFT*iFilteredN*sizeof(double));
    m_baAvrIm.resize(NFFT*iFilteredN*sizeof(double));
    m_baAvrM1.resize(NFFT*iFilteredN*sizeof(double));
    m_baAvrM2.resize(NFFT*iFilteredN*sizeof(double));
    m_baAvrM3.resize(NFFT*iFilteredN*sizeof(double));
    m_baAvrM4.resize(NFFT*iFilteredN*sizeof(double));

    double *pAvrRe=(double *)m_baAvrRe.data();
    double *pAvrIm=(double *)m_baAvrIm.data();
    double *pAvrM1=(double *)m_baAvrM1.data();
    double *pAvrM2=(double *)m_baAvrM2.data();
    double *pAvrM3=(double *)m_baAvrM3.data();
    double *pAvrM4=(double *)m_baAvrM4.data();

    for (int iDelay=0; iDelay<iFilteredN; iDelay++) {
        for (int kDoppler=0; kDoppler<NFFT; kDoppler++) {
            struct intfMapRecord imRecord;
            uBytesRead = m_qfIntfMap.read((char*)&imRecord,sizeof(struct intfMapRecord));
            if (uBytesRead != sizeof(struct intfMapRecord)) {
                qDebug() << "struct intfMapRecord read failed" << endl;
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
    m_qfIntfMap.close();
    return iRet;
}
//======================================================================================================
//
//======================================================================================================
void QPoi::resetRandomNumberGenerators() {
    // initialize seed of stdlib.h pseudo-random numbers (iSeed is read-only)
    unsigned int iSeed=1;
    std::srand(iSeed);

    // initialize seed of Numerical Recipes Gaussian random numbers (m_idum is read-write)
    this->m_idum=-1;
    NR::gasdev(this->m_idum);
}
//======================================================================================================
//
//======================================================================================================
bool QPoi::detectTargets(QByteArray &baSamples, QByteArray &baStrTargets, int &nTargets) {
    bool retVal=false;

    int beamCountsNum = Np*NT_;
    int iDataSize = baSamples.size();
    int iArrElemCount = 2*Np*NT_;
    int iSizeOfComplex = 2*sizeof(qint16);
    if (iDataSize!=beamCountsNum*iSizeOfComplex) {
        qDebug() << "iDataSize!=beamCountsNum*iSizeOfComplex";
        return retVal;
    }
    qint16 *pData = (qint16*) baSamples.data();

    QByteArray baDopplerRepresentation = dopplerRepresentation(pData, iArrElemCount);
    if (baDopplerRepresentation.isEmpty()) {
        qDebug() << "baDopplerRepresentation.isEmpty()";
        return retVal;
    }
    if (baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)) {
        qDebug() << "baDopplerRepresentation.size()<2*iFilteredN*NFFT*sizeof(double)";
        return retVal;
    }
    double *pDopplerData=(double*)baDopplerRepresentation.data();

    // phy scales
    double dVLight=300.0; // m/usec
    double dDoppFmin=1.0e0/(NT*m_dTs)/NFFT; // This is smallest Doppler frequency (MHz) possible for this sampling interval dTs
    double dVelCoef = 1.0e6*dVLight/2/m_dCarrierF; // Increment of target velocity (m/s) per 1 MHz of Doppler shift
    dVelCoef = dVelCoef*dDoppFmin; // m/s per frequency count (the "smallest" Doppler frequency possible)
    double dDistCoef = m_dTs*dVLight/2; // m per sample (target distance increment per sampling interval dTs)

    //================================================================================================================================
    //  ok, at this point the Doppler representation (Re,Im)=(pDopplerData[2*(k*iFilteredN+i)],pDopplerData[2*(k*iFilteredN+i)+1])
    //  where k = 0,...,(NFFT-1) is Doppler frequency index, currently 20191016 this is 0,...,1023
    //  and   i = 0, ..., (iFilteredN-1) is delay index, currently 20191016 this is 0,...,72
    //================================================================================================================================

    // initialize number of candidates nc
    int nc=0;

    // calculate globalmodulus squared of signal
    double *pAvrRe=(double *)m_pIntfMap->m_baAvrRe.data();
    double *pAvrIm=(double *)m_pIntfMap->m_baAvrIm.data();
    // double *pAvrM1=(double *)m_baAvrM1.data();
    double *pAvrM2=(double *)m_pIntfMap->m_baAvrM2.data();
    // double *pAvrM3=(double *)m_baAvrM3.data();
    //double *pAvrM4=(double *)m_baAvrM4.data();
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
                qDebug() << "dAvrM2 < 0.0e0";
                return retVal;
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

            if (dFrac > m_dThreshold && dProb < m_dFalseAlarmProb) {
                nc++;
                kc.append(kDoppler);
                lc.append(iDelay);
                y2mc.append(pY2M[idx]);
            }
        }
    }
    if (!nc) {
        // qDebug() << "No candidates!";
        return retVal;
    }

    // multiple targets resolution
    int ntg=0;
    QList<int> tgs;
    QList<double> ktg,ltg;
    QList<double> y2mtg;
    QList<double> y2mtg_sum;
    QList<int> lmtg,kmtg;
    QMultiMap<int,int> qmmTgStruct;
    int tglmax=8;

    for (int ic=0; ic<nc; ic++) {  // loop over all candidates - resolution elements (l,k) with threshold reached
        bool bFlag=false;          // found candidates attributed to running target
        for (int itg=0; itg<ntg; itg++) {  // loop over existing targets itg
            int ktg_=ktg.at(itg);          // target mass center (Doppler)
            int ltg_=ltg.at(itg);          // target mass center (delay)
            int kc_=kc.at(ic);             // resolution element along Doppler dimension for candidate ic
            int lc_=lc.at(ic);             // resolution element along delay for candidate ic
            // test delay difference
            if (abs(lc_-ltg_)>tglmax) continue;     // if candidate delay is too far from target, then skip
            // test Doppler channel difference
            double kdiff=kc_-ktg_;
            if (kdiff>NFFT/2) kdiff-=NFFT;
            if (kdiff<-NFFT/2) kdiff+=NFFT;
            if (abs(kdiff)>1) continue;             // if Doppler channel difference is greater than 1, then skip
            // candidate associated with itg
            bFlag=true;
            int tgs_=tgs.at(itg)+1;                 // candidate belongs to target
            y2mtg_sum[itg]+=y2mc.at(ic);            // add to target power
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
        if (bFlag) continue; // candidate belongs to existing target, hence continue
        // create new target from current candidate
        ltg.append(lc.at(ic));
        ktg.append(kc.at(ic));
        tgs.append(1);
        y2mtg.append(y2mc.at(ic));
        y2mtg_sum.append(y2mc.at(ic));
        lmtg.append(lc.at(ic));
        kmtg.append(kc.at(ic));
        qmmTgStruct.insertMulti(ntg,ic);
        ntg=ntg+1;
    }

    bool bTargetsListed=false;
    for (int itg=0; itg<ntg; itg++) {
        double kDoppler=ktg.at(itg);
        double iDelay=ltg.at(itg);
        double dTgVel;
        if (kDoppler >= NFFT/2) {
            dTgVel=dVelCoef*(kDoppler-NFFT);
        }
        else {
            dTgVel=dVelCoef*kDoppler;
        }
        double dTgDist = iDelay*dDistCoef;
        baStrTargets.clear();   // return array of struct sTarget
        nTargets=0;             // return number of valid targets
        if (tgs.at(itg)>5) {
            bTargetsListed=true;
            //double dTgPower = dY2M;
            baStrTargets.resize(baStrTargets.size()+sizeof(struct sTarget));
            struct sTarget *pStrTarget = (struct sTarget *)baStrTargets.data()+nTargets;
            nTargets++;
            pStrTarget->uCandNum=tgs.at(itg);
            pStrTarget->qpf_wei=QPointF(dTgDist,dTgVel);
            pStrTarget->y2mc_sum=y2mtg_sum[itg];
            pStrTarget->qp_rep=QPoint(lmtg[itg],kmtg[itg]);
            pStrTarget->y2mc_rep=y2mtg[itg];
        }
    }
    // qDebug() << "At exit bTargetsListed = " << bTargetsListed;
    return bTargetsListed;
}
//======================================================================================================
//
//======================================================================================================
QByteArray QPoi::dopplerRepresentation(qint16 *pData, unsigned int iArrElemCount) {
    if (iArrElemCount!=2*NT_*Np || NT_<Ntau || NT<NT_ || iFilteredN!=NT_-Ntau+1) return QByteArray();
    unsigned int i,j,k;
    // output text stream for debugging
    // QTextStream tsStdOut(stdout);

    // loop over periods, in-place calculation
    // after matched filter the received signal array length is iFilteredN samples
    QByteArray baDataFiltered(2*Np*iFilteredN*sizeof(double),0);
    double *pDataFiltered=(double *)baDataFiltered.data();
    for (k=0; k<Np; k++) {
        // loop over one period (recorded fragment) 0...NT_-1
        for (i=0; i<iFilteredN; i++) { // index within one period
            qint32 iRe=0,iIm=0; // running cumulants
            int idx; // index of input array pData: idx=2*(k*NT_),...,2*(k*NT_+iFilteredN-1)
            idx=2*(k*NT_+i); // k*NT_ - offset of period beginning; i - offset of filter output: i=0,1,...,(NT_-Ntau)
            for (int itau=0; itau<Ntau; itau++) { // summation over pulse duration: itau=0,1,...,(Ntau-1)
                iRe+=pData[idx+2*itau];   // the first array index is 2*(k*NT_)
                iIm+=pData[idx+2*itau+1]; // the last array index is 2*(k*NT_+iFilteredN-1)+2*(Ntau-1)+1 = 2*(k*NT_+NT_)-1
                // Thus within one period we span (2*NT_) pData array elements: 2*(k*NT_)+0, ..., 2*(k*NT_+NT_)-1
            }
            idx=2*(k*iFilteredN+i); // last index i = (NT_-Ntau+1)-1=NT_-Ntau
            // Thus within one period we produce 2*(NT_-Ntau+1) pDataFiltered array elements of type double
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
//====================================================================
//
//====================================================================
void QPoi::avevar_poi(Vec_I_DP &data, DP &ave, DP &var) {
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
//====================================================================
//
//====================================================================
void QPoi::ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob) {
    DP var_noise,var_signal,ave_noise,ave_signal;

    int n_noise=data_noise.size();
    int n_signal=data_signal.size();
    if (n_signal!=2) qFatal("n_signal!=2");
    avevar_poi(data_noise,ave_noise,var_noise);
    avevar_poi(data_signal,ave_signal,var_signal);
    f=var_signal/var_noise; // signal-to-noise
    prob = NR::betai(0.5*n_noise,0.5*n_signal,n_noise/(n_noise+n_signal*f));
}
