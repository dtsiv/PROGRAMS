#include "qsimumgr.h"
#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

#include <cmath>

// #define PHASE_COHERENCE

QFile qfStrobeParams;

#ifdef PHASE_COHERENCE
double getArg(double dRe, double dIm, bool &bOk) {
    bOk = false;
    double dEPS=1.0e-40;
    double dR2=dRe*dRe+dIm*dIm;
    if (dR2 < dEPS) return 0.0e0;
    bOk=true;
    return atan2(dIm,dRe);
}
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSimuMgr::QSimuMgr(QIndicatorWindow *pOwner) :
          m_pOwner(pOwner)
        , m_pPoi(NULL) {

    // local copies for convenience
    m_pPoi = m_pOwner->m_pPoi;
    m_pSqlModel = m_pOwner->m_pSqlModel;
    m_pTargetsMap = m_pOwner->m_pTargetsMap;

    m_qfPeleng.setFileName("peleng.dat");
    m_qfPeleng.open(QIODevice::WriteOnly);
    m_tsPeleng.setDevice(&m_qfPeleng);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSimuMgr::~QSimuMgr() {
    if (m_qfPeleng.isOpen()) m_qfPeleng.close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSimuMgr::processStrob() {
    quint64 iRecId;
    int iStrob;
    qint64 iTimeStamp;
    static bool bStarted=false;
    static double dStartTime=0;

    // map members for convenience
    QByteArray &baStructStobeData  = m_pPoi->m_baStructStobeData;
    quint32 &uFileVer              = m_pPoi->m_uFileVer;

    // get next strob record guid from DB
    if (!m_pSqlModel->getStrobRecord(iRecId, iStrob, m_pPoi->iBeamCountsNum, iTimeStamp, baStructStobeData, uFileVer)) {
        if (!bStarted) qDebug() << "getStrobRecord failed";
        qDebug() << "getStrobRecord failed - stopping timer";
        m_pOwner->m_simulationTimer.stop();
        return;
    }
    // qDebug() << "processStrob(): iStrob = " << iStrob;

    // check file version
    if (uFileVer!=REG::FORMAT_VERSION) {
        throw RmoException(QString("QSimuMgr::processStrob(): getStrobRecord failed - wrong uFileVer = %1")
                           .arg(uFileVer));
        return;
    }

    // update m_pPoi with current ACM::STROBE_DATA struct
    ACM::STROBE_DATA *pStructStrobeData = (ACM::STROBE_DATA*)baStructStobeData.data();
    CHR::STROBE_HEADER *pStructStrobeHeader = (CHR::STROBE_HEADER*)(&pStructStrobeData->header);
    if (!bStarted) {
        qDebug() << "first call to processStrob()";
        if (!m_pPoi->updateStrobeParams(
                        pStructStrobeHeader->pCount,        // количество импульсов в стробе (число периодов = 1024)
                        pStructStrobeHeader->pDuration,     // Длительность импульса (число выборок в одном импульсе = 8)
                        pStructStrobeHeader->pPeriod,       // Полное число выборок в одном периоде (повторения импульсов = 200)
                        pStructStrobeHeader->distance,      // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
                        pStructStrobeData->beamCountsNum)) {
            throw RmoException("QSimuMgr::processStrob(): updateStrobeParams() failed");
            return;
        }

        #if 0
//========================================================================================================================
        // strobe params check
        if (m_pPoi->m_bUseLog) {
            // qDebug() << "Sizeof (struct ACM::STROBE_DATA) = " << sizeof(struct ACM::STROBE_DATA);
            if (!qfStrobeParams.isOpen()) {
                qfStrobeParams.setFileName(QDir::current().absolutePath()+"/stroblog.txt");
                if (!qfStrobeParams.open(QIODevice::WriteOnly)) qDebug() << "qfStrobeParams.open() failed";
                tsStrobeParams.setDevice(&qfStrobeParams);
                tsStrobeParams
                        << "strobeNo" << "\t"
                        << "execTime" << "\t"                       // presumably execTime in samples (0.12us)
                        << "timeParams" << "\t"
                        << "inclBeta" << "\t"
                        << "inclEpsilon" << "\t"
                        << "beamBeta" << "\t"
                        << "beamEpsilon" << "\t"
                        << "sensorBeta" << "\t"
                        << "sensorEpsilon" << "\t"
                        << "velocity" << "\t"
                        << "signalID" << "\t"
                        << "flags" << "\t"
                        << endl;
            }
        }
//========================================================================================================================
        #endif
    }
    else if (!m_pPoi->checkStrobeParams(
                    pStructStrobeHeader->pCount,        // количество импульсов в стробе (число периодов = 1024)
                    pStructStrobeHeader->pDuration,     // Длительность импульса (число выборок в одном импульсе = 8)
                    pStructStrobeHeader->pPeriod,       // Полное число выборок в одном периоде (повторения импульсов = 200)
                    pStructStrobeHeader->distance,      // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
                    pStructStrobeData->beamCountsNum)) {
        throw RmoException("QSimuMgr::processStrob(): checkStrobeParams() failed");
        return;
    }

    #if 0
//========================================================================================================================
    // strobe params check
    if (m_pPoi->m_bUseLog) {
        if (qfStrobeParams.isOpen()) {
            CHR::BEAM_POS *pBeamPos = &pStructStrobeData->beamPos;
            // output to stroblog.txt
            tsStrobeParams
                    << pStructStrobeHeader->strobeNo << "\t"
                    << pStructStrobeHeader->execTime << "\t"       // presumably execTime in samples (0.12us)
                    << pStructStrobeData->timeParams << "\t"
                    << pStructStrobeData->inclBeta << "\t"
                    << pStructStrobeData->inclEpsilon << "\t"
                    << pBeamPos->beamBeta << "\t"
                    << pBeamPos->beamEpsilon << "\t"
                    << pBeamPos->sensorBeta << "\t"
                    << pBeamPos->sensorEpsilon << "\t"
                    << pStructStrobeData->velocity << "\t"
                    << pStructStrobeHeader->signalID << "\t"
                    << pStructStrobeHeader->flags << "\t"
                    << endl;
        }
    }
//========================================================================================================================
    #endif

    // number of points in FFT
    if (!m_pPoi->updateNFFT()) {
        throw RmoException("QSimuMgr::processStrob(): updateNFFT() failed");
        return;
    }

    // read noise map from file
    if (m_pPoi->m_bUseNoiseMap) {
        if (!bStarted) {
            bool bZeroFill=true;
            m_pPoi->m_pNoiseMap->resizeNoiseAvrArrays(m_pPoi->iFilteredN, m_pPoi->NFFT,bZeroFill);
            int iRet=m_pPoi->m_pNoiseMap->readNoiseMap(m_pPoi->iFilteredN, m_pPoi->NT_, m_pPoi->Np, m_pPoi->NFFT);
            if (iRet) {
                qDebug() << "readNoiseMap() returned: " << iRet;
            }
        }
    }

    // raw data array
    QByteArray baBeamsSumDP;
    // iBeamCountsNum = NT_ * Np   // количество комплексных (Re, Im) отсчётов в стробе
    // ============================================================================================
    // Np         // количество импульсов в стробе (число периодов = 1024)
    // Ntau       // Длительность импульса (число выборок в одном импульсе = 8)
    // NT         // Полное число выборок в одном периоде (повторения импульсов = 200)
    // NT_        // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
    baBeamsSumDP.resize(m_pPoi->iBeamCountsNum*2*sizeof(double));
    double *pSum = (double *)baBeamsSumDP.data();
    for (int i=0; i<2*m_pPoi->iBeamCountsNum; i++) { pSum[i]=0.0e0; }

    // List of beams
    QByteArray pbaBeamDataDP[QPOI_NUMBER_OF_BEAMS];

    // detect all targets for all beams
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) {
        QByteArray baBeam;
        baBeam.clear();
        if (!m_pSqlModel->getBeamData(iRecId, iBeam, baBeam)) {
            qDebug() << "getBeamData failed";
            m_pOwner->m_simulationTimer.stop();
            return;
        }
        if (baBeam.size() != m_pPoi->iBeamCountsNum*2*sizeof(qint16)) {
            qDebug() << "getBeamData: baBeam.size() != m_pPoi->iBeamCountsNum*2*sizeof(qint16)";
            m_pOwner->m_simulationTimer.stop();
            return;
        }
        qint16 *pBeam = (qint16 *)baBeam.data();
        pbaBeamDataDP[iBeam]=QByteArray(baBeamsSumDP.size(),(char)0);
        double *pBeamDP = (double *)pbaBeamDataDP[iBeam].data();
        for (int i=0; i<2*m_pPoi->iBeamCountsNum; i++) {
            pBeamDP[i] = 1.0e0*pBeam[i];
            pSum[i] += pBeamDP[i];
        }
    }

    {
    //     bStarted = true;
        int iStrobNo = pStructStrobeHeader->strobeNo;
        if (iStrobNo%100 == 0) qDebug() << "iStrobNo = " << iStrobNo;
    //     // if (m_pOwner->m_pTargetsMap->m_uTimerMSecs == 0) QTimer::singleShot(0,m_pOwner,SLOT(onSimulationTimeout()));
    //     return;
    }

    // detect targets
    QByteArray baStrTargets;
    int nTargets;
    // int iStrobNo = pStructStrobeHeader->strobeNo;
    if (!m_pPoi->detectTargets(baBeamsSumDP, baStrTargets, nTargets)) {
        // qDebug() << "detectTargets failed: " << iStrob << " " << iBeam;
        bStarted = true;
        return;
    }
    if (baStrTargets.size()!=nTargets*sizeof(struct QPoi::sTarget)) {
        qDebug() << "struct size mismatch: " << baStrTargets.size() << " " << nTargets*sizeof(struct QPoi::sTarget);
        m_pOwner->m_simulationTimer.stop();
        return;
    }
    for (int iTarget = 0; iTarget < nTargets; iTarget++) {
        struct QPoi::sTarget *pTarData = (struct QPoi::sTarget *)baStrTargets.data()+iTarget;

        bool bOk;
        double dBeamAmplRe[QPOI_NUMBER_OF_BEAMS], dBeamAmplIm[QPOI_NUMBER_OF_BEAMS];
        // Max signal amplitude for target iTarget found at (iDelay, kDoppler)
        int iDelay = pTarData->qp_rep.x();
        int kDoppler = pTarData->qp_rep.y();
        bOk = m_pPoi->getPointDopplerRep(iDelay, kDoppler, pbaBeamDataDP, dBeamAmplRe, dBeamAmplIm);

        // calculate angles from signal amplitudes for beams
        double dAzimuth, dElevation; // angles in radians
        bool bAnglesOk = m_pPoi->getAngles(dBeamAmplRe, dBeamAmplIm, dAzimuth, dElevation);

        // debug printout
        {
            double dSumRe = dBeamAmplRe[0] + dBeamAmplRe[1] + dBeamAmplRe[2] + dBeamAmplRe[3];
            double dSumIm = dBeamAmplIm[0] + dBeamAmplIm[1] + dBeamAmplIm[2] + dBeamAmplIm[3];
            double dSum2 = dSumRe*dSumRe + dSumIm*dSumIm;

            double dSumAzRe = dBeamAmplRe[0] + dBeamAmplRe[2] - dBeamAmplRe[1] - dBeamAmplRe[3];
            double dSumAzIm = dBeamAmplIm[0] + dBeamAmplIm[2] - dBeamAmplIm[1] - dBeamAmplIm[3];

            double dSumElRe = dBeamAmplRe[2] + dBeamAmplRe[3] - dBeamAmplRe[0] - dBeamAmplRe[1];
            double dSumElIm = dBeamAmplIm[2] + dBeamAmplIm[3] - dBeamAmplIm[0] - dBeamAmplIm[1];

            double dRatioAz = dSumAzRe*dSumRe+dSumAzIm*dSumIm; dRatioAz/=dSum2;
            double dRatioEl = dSumElRe*dSumRe+dSumElIm*dSumIm; dRatioEl/=dSum2;

            CHR::BEAM_POS *pBeamPos = &pStructStrobeData->beamPos;
            double dPI = 3.14159265e0;
            double dAzimuthScan = pBeamPos->beamBeta*180.0e0/32768;
            double dElevationScan = pBeamPos->beamEpsilon*180.0e0/32768;

            if (!bStarted) dStartTime = (pStructStrobeHeader->execTime) * (m_pPoi->m_dTs);

#ifdef PHASE_COHERENCE
            // phase coherence ==================================================================
            bool bOk0,bOk1,bOk2,bOk3,bOkAvr;
            double dPhi0=getArg(dBeamAmplRe[0],dBeamAmplIm[0],bOk0);
            double dPhi1=getArg(dBeamAmplRe[1],dBeamAmplIm[1],bOk1);
            double dPhi2=getArg(dBeamAmplRe[2],dBeamAmplIm[2],bOk2);
            double dPhi3=getArg(dBeamAmplRe[3],dBeamAmplIm[3],bOk3);
            double xC=cos(dPhi0)+cos(dPhi1)+cos(dPhi2)+cos(dPhi3);
            double yC=sin(dPhi0)+sin(dPhi1)+sin(dPhi2)+sin(dPhi3);
            double dPhiAvr=getArg(xC,yC,bOk0);
            if (!bOk0 || !bOk1 || !bOk2 || !bOk3 || !bOkAvr) qDebug() << "getArg() failed for strob: " << pStructStrobeHeader->strobeNo;
            dPhi0=180.0e0/dPI*(dPhi0-dPhiAvr);
            dPhi1=180.0e0/dPI*(dPhi1-dPhiAvr);
            dPhi2=180.0e0/dPI*(dPhi2-dPhiAvr);
            dPhi3=180.0e0/dPI*(dPhi3-dPhiAvr);
            dPhi0=dPhi0-360.0e0*qRound(dPhi0/360.0e0);
            dPhi1=dPhi1-360.0e0*qRound(dPhi1/360.0e0);
            dPhi2=dPhi2-360.0e0*qRound(dPhi2/360.0e0);
            dPhi3=dPhi3-360.0e0*qRound(dPhi3/360.0e0);
            // amplitude method
            double dAmplRatioAz,dAmplRatioEl;
            {
                double dA0,dA1,dA2,dA3;
                dA0=dBeamAmplRe[0]*dBeamAmplRe[0]+dBeamAmplIm[0]*dBeamAmplIm[0];
                dA0=sqrt(dA0);
                dA1=dBeamAmplRe[1]*dBeamAmplRe[1]+dBeamAmplIm[1]*dBeamAmplIm[1];
                dA1=sqrt(dA1);
                dA2=dBeamAmplRe[2]*dBeamAmplRe[2]+dBeamAmplIm[2]*dBeamAmplIm[2];
                dA2=sqrt(dA2);
                dA3=dBeamAmplRe[3]*dBeamAmplRe[3]+dBeamAmplIm[3]*dBeamAmplIm[3];
                dA3=sqrt(dA3);
                double dAsum = dA0 + dA1 + dA2 + dA3;
                // Azimuth
                dAmplRatioAz = (dA0 + dA2 - dA1 - dA3)/dAsum;
                // Elevation
                dAmplRatioEl = (dA2 + dA3 - dA0 - dA1)/dAsum;
            }
            if (qRound(dElevationScan) == 2)
            // end of phase coherence ==================================================================
#endif

            m_tsPeleng << ((pStructStrobeHeader->execTime) * (m_pPoi->m_dTs) - dStartTime) * 1.0e-6
                << "\t" << dAzimuthScan
                << "\t" << dElevationScan
                << "\t" << dAzimuth*180.0e0/dPI
                << "\t" << dElevation*180.0e0/dPI
                << "\t" << dAzimuthScan + dAzimuth*180.0e0/dPI
                << "\t" << dElevationScan + dElevation*180.0e0/dPI
                << "\t" << pStructStrobeHeader->strobeNo
                << "\t" << pTarData->qpf_wei.x()
                << "\t" << pTarData->qpf_wei.y()
                << "\t" << (int)bAnglesOk
#ifdef PHASE_COHERENCE
                // phase coherence ==================================================================
                << "\t" << (int)dPhi0
                << "\t" << (int)dPhi1
                << "\t" << (int)dPhi2
                << "\t" << (int)dPhi3
                // amplitude method
                // << "\t" <<  dAmplRatioAz
                // << "\t" <<  dAmplRatioEl
                // end of phase coherence ==================================================================
#endif
                << endl;
        }

        // prepare target marker
        QString qsTempl("S:%1\ndB:%2\nT:%3\nAz:%4\nEl:%5");
        QPointF qpfTar = pTarData->qpf_wei;
        double dGlobal_dB = m_pPoi->m_pNoiseMap->m_dGlobal_dB;
        double dM2Tar=10.0*log(pTarData->y2mc_rep)/log(10.0e0) - dGlobal_dB;
        double dTime = ((pStructStrobeHeader->execTime) * (m_pPoi->m_dTs) - dStartTime) * 1.0e-6;
        QString qsLegend;
        if (bAnglesOk) {
            CHR::BEAM_POS *pBeamPos = &pStructStrobeData->beamPos;
            double dPI = 3.14159265e0;
            double dAzimuthScan = pBeamPos->beamBeta*180.0e0/32768;
            double dElevationScan = pBeamPos->beamEpsilon*180.0e0/32768;
            dAzimuth = dAzimuthScan + dAzimuth*180.0e0/dPI;
            dElevation = dElevationScan + dElevation*180.0e0/dPI;
            qsLegend = qsTempl.arg(iStrob)
                              .arg(dM2Tar,0,'f',0)
                              .arg(dTime,0,'f',1)
                              .arg(dAzimuth,0,'f',2)
                              .arg(dElevation,0,'f',2);
        }
        else {
            qsLegend = qsTempl.arg(iStrob)
                              .arg(dM2Tar,0,'f',0)
                              .arg(dTime,0,'f',1)
                              .arg("N/A")
                              .arg("N/A");
        }
        QTargetMarker *pTarMark = new QTargetMarker(qpfTar,qsLegend);
        m_pTargetsMap->addTargetMarker(pTarMark);
    }
    if (!bStarted) qDebug() << "processStrob() finished -> bStarted = true";
    bStarted = true;
    // if (m_pOwner->m_pTargetsMap->m_uTimerMSecs == 0) QTimer::singleShot(0,m_pOwner,SLOT(onSimulationTimeout()));
    return;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::onSimulationTimeout() {
    if (m_pSimuMgr) m_pSimuMgr->processStrob();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::toggleTimer() {
    bool bIsActive = m_simulationTimer.isActive();
    if (bIsActive) {
        m_simulationTimer.stop();
    }
    else {
        m_simulationTimer.start();
    }
}
