#include "qnoisemapmgr.h"
#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

#include "windows.h"

double dNoiseMapGenerateProgressBarStep=(double)GENERATE_PROGRESS_BAR_STEP/GENERATE_PROGRESS_BAR_MAX;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QNoiseMapMgr::QNoiseMapMgr(QIndicatorWindow *pOwner) :
                m_pOwner(pOwner)
              , iNumberOfBeams(4)
              , iSizeOfComplex(2*sizeof(qint16))
              , m_bStarted(false) {
    // map members for convenience
    m_pPoi                     = m_pOwner->m_pPoi;
    m_pNoiseMap                = m_pPoi->m_pNoiseMap;
    m_pSqlModel                = m_pOwner->m_pSqlModel;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QNoiseMapMgr::startGenerateNoiseMap(QObject *pSender) {
    qDebug() << "startGenerateNoiseMap(): stopping simulation timer";
    // get access to QPropPages dynamic object (!take care - only exists while dlg)
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pSender);
    m_pOwner->m_simulationTimer.stop();
    m_pOwner->m_bGenerateNoiseMapInProgress =true;
    pPropPages->m_ppbAccept->setEnabled(false);
    pPropPages->m_ppbGenerateNoiseMap->setEnabled(false);
    pPropPages->m_ppbarGenerateNoiseMap->setMaximum(GENERATE_PROGRESS_BAR_MAX);

    // ensure progress bar updates
    QObject::connect(m_pOwner,SIGNAL(updateGenerateNoiseMapProgressBar(double)),pPropPages,SIGNAL(updateGenerateNoiseMapProgressBar(double)));

    // get current NoiseMapFile name and check if it exists and isReadbale
    QString qsNoiseMapFile = pPropPages->m_pleNoiseMapFName->text();
    QFileInfo fiNoiseMapFile(qsNoiseMapFile);

    // proceed with noise map generation
    if (m_pSqlModel->isDBOpen()) {
        m_pSqlModel->closeDatabase();
        m_pOwner->lbStatusArea->setText(CONN_STATUS_DISCONN);
    }
    updateGenerateNoiseMapProgressBar(true);
    if (!m_pSqlModel->openDatabase()) {
        throw RmoException(QString("NoiseMap: openDatabase() failed"));
        return;
    }
    m_pOwner->lbStatusArea->setText(CONN_STATUS_SQLITE);

    // process pending events
    int iMaxMSecs=500;
    QCoreApplication::processEvents(QEventLoop::AllEvents,iMaxMSecs);

    // total number of strobes
    if (!m_pSqlModel->getTotStrobes(m_pNoiseMap->m_nStrobsTotal) || m_pNoiseMap->m_nStrobsTotal <= 0) {
        throw RmoException(QString("NoiseMap: getTotStrobes() failed"));
        return;
    }

    // query strobes
    if (!m_pSqlModel->execQuery()) {
        throw RmoException(QString("NoiseMap: execQuery() failed"));
        return;
    }
    quint64 iRecId;
    int iStrob,iBeamCountNum;
    qint64 iTimeStamp;
    quint32 uFileVer;
    QByteArray baStructStrobeData;
    while (m_pSqlModel->getStrobRecord(iRecId,iStrob,iBeamCountNum,iTimeStamp,baStructStrobeData,uFileVer)) {
        // update m_pPoi with current ACM::STROBE_DATA struct
        if (uFileVer!=REG::FORMAT_VERSION) {
            throw RmoException(QString("NoiseMap: uFileVer!=REG::FORMAT_VERSION (%1 != %2)")
                               .arg(uFileVer).arg(REG::FORMAT_VERSION));
            return;
        }

        ACM::STROBE_DATA *pStructStrobeData = (ACM::STROBE_DATA*)baStructStrobeData.data();
        CHR::STROBE_HEADER *pStructStrobeHeader = (CHR::STROBE_HEADER*)(&pStructStrobeData->header);

        if (!m_bStarted) {
            // update QPoi params
            if (!m_pPoi->updateStrobeParams(
                           pStructStrobeHeader->pCount,        // количество импульсов в стробе (число периодов = 1024)
                           pStructStrobeHeader->pDuration,     // Длительность импульса (число выборок в одном импульсе = 8)
                           pStructStrobeHeader->pPeriod,       // Полное число выборок в одном периоде (повторения импульсов = 200)
                           pStructStrobeHeader->distance,      // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
                           pStructStrobeData->beamCountsNum)) {
                throw RmoException("NoiseMap: updateStrobeParams() failed");
                return;
            }
            // number of points in FFT
            if (!m_pPoi->updateNFFT()) {
                throw RmoException("NoiseMap: m_pPoi->updateNFFT() failed");
                return;
            }

            // reset arrays of average values
            bool bZeroFill = true;
            m_pNoiseMap->resizeNoiseAvrArrays(m_pPoi->iFilteredN,m_pPoi->NFFT,bZeroFill);

            // set the started flag
            m_bStarted = true;
        }
        else if (!m_pPoi->checkStrobeParams(
                     pStructStrobeHeader->pCount,        // количество импульсов в стробе (число периодов = 1024)
                     pStructStrobeHeader->pDuration,     // Длительность импульса (число выборок в одном импульсе = 8)
                     pStructStrobeHeader->pPeriod,       // Полное число выборок в одном периоде (повторения импульсов = 200)
                     pStructStrobeHeader->distance,      // Дистанция приема (число выборок, регистрируемых в одном периоде = 80)
                     pStructStrobeData->beamCountsNum)) {
            throw RmoException(QString("NoiseMap: checkStrobeParams() failed"));
            return;
        }

        // List of beams (qint16,qint16) format
        QByteArray pbaBeamData[QPOI_NUMBER_OF_BEAMS];

        // raw data array
        for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) {
            pbaBeamData[iBeam].clear();
            if (!m_pSqlModel->getBeamData(iRecId, iBeam, pbaBeamData[iBeam])) {
                qDebug() << "NoiseMap: getBeamData() failed";
                throw RmoException(QString("NoiseMap: getBeamData() failed"));
                return;
            }
            if (pbaBeamData[iBeam].size() != m_pPoi->iBeamCountsNum*2*sizeof(qint16)) {
                qDebug() << "getBeamData: baBeam.size() != m_pPoi->iBeamCountsNum*2*sizeof(qint16)";
                throw RmoException(QString("getBeamData: baBeam.size() != m_pPoi->iBeamCountsNum*2*sizeof(qint16)"));
                return;
            }
        }

        // append strob (list of beams) to noise map
        if (!m_pPoi->appendStrobToNoiseMap(pbaBeamData)) {
            throw RmoException(QString("NoiseMap: m_pPoi->appendStrobToNoiseMap() failed"));
            return;
        }

        // progress in strobes number
        m_pNoiseMap->m_nStrobs++;
        if (m_pNoiseMap->m_nStrobs>m_pNoiseMap->m_nStrobsTotal) {
            throw RmoException(QString("NoiseMap: m_pNoiseMap->m_nStrobs>m_pNoiseMap->m_nStrobsTotal"));
            return;
        }
        // qDebug() << m_pNoiseMap->m_nStrobs;
        updateGenerateNoiseMapProgressBar();
    }

    // calculate resulting average values
    if (!m_pPoi->calcAverages()) {
        throw RmoException(QString("NoiseMap: m_pPoi->calcAverages() failed"));
        return;
    }

    // write results to file noisemap.dat
    m_pPoi->writeNoiseMapFile(fiNoiseMapFile.absoluteFilePath());

    // reset controls&flags to original state
    pPropPages->m_ppbAccept->setEnabled(true);
    pPropPages->m_ppbGenerateNoiseMap->setEnabled(true);
    pPropPages->m_ppbarGenerateNoiseMap->setEnabled(true);
    m_pOwner->m_bGenerateNoiseMapInProgress=false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QNoiseMapMgr::updateGenerateNoiseMapProgressBar(bool bReset /* =false */) {
    static int iCurr=0,iPrev=0;
    if (bReset) {
        iCurr=iPrev=0;
        emit m_pOwner->updateGenerateNoiseMapProgressBar(iCurr);
        return;
    }
    // qDebug() << "m_pNoiseMap->getProgress() = " << m_pNoiseMap->getProgress();
    double dProgress = m_pNoiseMap->getProgress();
    iCurr = qRound(dProgress/dNoiseMapGenerateProgressBarStep);
    // qDebug() << "iCurr = " << iCurr << " dProgress = " << dProgress << " dNoiseMapGenerateProgressBarStep = " << dNoiseMapGenerateProgressBarStep;
    if (iCurr>iPrev) {
        iPrev=iCurr;
        emit m_pOwner->updateGenerateNoiseMapProgressBar(iCurr*dNoiseMapGenerateProgressBarStep);
    }
    int iMaxMSecs=500;
    QCoreApplication::processEvents(QEventLoop::AllEvents,iMaxMSecs);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::onNoiseMapFileGenerate() {
    if (m_pNoiseMapMgr) {
        m_pNoiseMapMgr->m_bStarted = false;
        m_pNoiseMapMgr->startGenerateNoiseMap(QObject::sender());
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::onUpdateGenerateNoiseMapProgressBar(double dCurr) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());
    int iMax=pPropPages->m_ppbarGenerateNoiseMap->maximum();
    int iVal = qRound(iMax*dCurr/GENERATE_PROGRESS_BAR_STEP)*GENERATE_PROGRESS_BAR_STEP;
    // qDebug() << "onUpdateGenerateNoiseMapProgressBar: dCurr=" << dCurr << " iVal=" << iVal ;
    pPropPages->m_ppbarGenerateNoiseMap->setValue(iVal);
}
