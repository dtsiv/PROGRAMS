#include "qpoi.h"

#include "nr.h"
using namespace std;

#define SKIP_ZERO_DOPPLER          0         // when noise map is used - no need to skip doppler channels
#define SKIP_ZERO_DELAY            0         // when noise map is used - no need to skip small delays
#define NOISE_AVERAGING_N          7

static int iCnt=0;

//======================================================================================================
//
//======================================================================================================
bool QPoi::detectTargets(QByteArray &baSamplesDP, QByteArray &baStrTargets, int &nTargets) {
    bool retVal=false;

    iCnt++;

    QByteArray baDopplerRepresentation = dopplerRepresentation(baSamplesDP);
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

    double *pAvrRe,*pAvrIm,*pAvrM1;
    // 20200502: if (iFilteredN!=NT_-Ntau+1) {  ---- changed to: iFilteredN!=NT_
    if (iFilteredN!=NT_) {
         // before 20200502: qDebug() << "QPoi::detectTargets(): iFilteredN!=NT_-Ntau+1";
        qDebug() << "QPoi::detectTargets(): iFilteredN!=NT_";
        return retVal;
    }
    bool bZeroFill=!m_bUseNoiseMap;
    m_pNoiseMap->resizeNoiseAvrArrays(iFilteredN, NFFT, bZeroFill);
    pAvrRe =(double *)m_pNoiseMap->m_baAvrRe.data();
    pAvrIm =(double *)m_pNoiseMap->m_baAvrIm.data();
    pAvrM1 =(double *)m_pNoiseMap->m_baAvrM1.data();
    // pAvrM2 =(double *)m_pNoiseMap->m_baAvrM2.data();
    // double *pAvrM3=(double *)m_baAvrM3.data();
    // double *pAvrM4=(double *)m_baAvrM4.data();
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

            // use pre-calculated noise map to rescale dRe, dIm
            if (m_bUseNoiseMap) {
                double dAvrRe=pAvrRe[kDoppler*iFilteredN+iDelay];
                double dAvrIm=pAvrIm[kDoppler*iFilteredN+iDelay];
                double dAvrM1=pAvrM1[kDoppler*iFilteredN+iDelay];
                // subtract average
                dRe-=dAvrRe;
                dIm-=dAvrIm;
                pDopplerData[idx] = dRe = dRe / dAvrM1;
                pDopplerData[idx+1] = dIm = dIm / dAvrM1;
            }

            // calculate signal in Doppler representation
            pY2M[idx]=dRe*dRe+dIm*dIm;
        }
    }

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
            int n_noise=dNoise.size();
            int n_signal=dSignal.size();
            if (n_signal!=2) qFatal("n_signal!=2");
            DP ave_noise,ave_signal;
            DP var_noise,var_signal;
            avevar_poi(dNoise,ave_noise,var_noise);
            avevar_poi(dSignal,ave_signal,var_signal);
            DP dFrac,dProb;
            dFrac=var_signal/var_noise; // signal-to-noise
            // Weinberg 2017 parameter
            double dTau = dFrac/(0.5*n_noise);
            // false alarm probability
            dProb = exp(-(0.5*n_noise)*log(1+dTau));
            //==========================================================================================
            // switch from full Fisher test
            // to simplified Winberg(2017) formula
            //==========================================================================================
            // void QPoi::ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob) {
            // DP var_noise,var_signal,ave_noise,ave_signal;
            //
            // int n_noise=data_noise.size();
            // int n_signal=data_signal.size();
            // if (n_signal!=2) qFatal("n_signal!=2");
            // avevar_poi(data_noise,ave_noise,var_noise);
            // avevar_poi(data_signal,ave_signal,var_signal);
            // f=var_signal/var_noise; // signal-to-noise
            // prob = NR::betai(0.5*n_noise,0.5*n_signal,n_noise/(n_noise+n_signal*f));
            //==========================================================================================
            // ftest_poi(dNoise,dSignal,dFrac,dProb);
            // dFracMax=(dFracMax>dFrac)?dFracMax:dFrac;
            // dProbMin=(dProbMin<dProb)?dProbMin:dProb;
            //==========================================================================================
            if (dProb < m_dFalseAlarmProb) {
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
        if (tgs.at(itg)>m_uTargSzThresh) {
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
            // qDebug() << "Tg y2mc_rep: " << y2mtg[itg] << " l: " << lmtg[itg] << " k: " << kmtg[itg];
        }
    }
    // qDebug() << "At exit bTargetsListed = " << bTargetsListed;
    return bTargetsListed;
}
