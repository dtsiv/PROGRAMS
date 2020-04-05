#include "qsimumgr.h"
#include "qindicatorwindow.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSimuMgr::QSimuMgr(QIndicatorWindow *pOwner) :
        m_pOwner(pOwner) {
    m_qfPeleng.setFileName("peleng.dat");
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
    int iBeamCountsNum;
    qint64 iTimeStamp;
    static bool bStarted=false;

    // map members for convenience
    QSqlModel *m_pSqlModel      = m_pOwner->m_pSqlModel;
    QTargetsMap *m_pTargetsMap  = m_pOwner->m_pTargetsMap;
    bool m_bUseShuffle          = m_pOwner->m_bUseShuffle;
    QPoi *m_pPoi                = m_pOwner->m_pPoi;

    // get next strob record guid from DB
    if (!m_pSqlModel->getStrobRecord(iRecId, iStrob, iBeamCountsNum, iTimeStamp)) {
        if (!bStarted) qDebug() << "getStrobRecord failed";
        qDebug() << "getStrobRecord failed - stopping timer";
        m_pOwner->m_simulationTimer.stop();
        return;
    }
    // raw data array
    QByteArray baSamples;
    // list of data records for beams
    QList<QByteArray> ql_baTarForBeam;
    QList<int> ql_nTargets;
    // detect all targets for all beams
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) {
        baSamples.clear();
        if (!m_pSqlModel->getBeamData(iRecId, iBeam, baSamples)) {
            qDebug() << "getBeamData failed";
            m_pOwner->m_simulationTimer.stop();
            return;
        }

        ql_baTarForBeam.append(QByteArray());
        ql_nTargets.append(0);
        if (ql_baTarForBeam.size()-1>iBeam || ql_nTargets.size()-1>iBeam) { // redundant check of list size
            qDebug() << "list size mismatch";
            m_pOwner->m_simulationTimer.stop();
            return;
        }
        QByteArray baStrTargets;
        int nTargets;
        if (!m_pPoi->detectTargets(baSamples, baStrTargets, nTargets)) {
            // qDebug() << "detectTargets failed: " << iStrob << " " << iBeam;
            continue;
        }
        ql_nTargets[iBeam] = nTargets;
        ql_baTarForBeam[iBeam] = baStrTargets;
        if (baStrTargets.size()!=nTargets*sizeof(struct QPoi::sTarget)) {
            qDebug() << "struct size mismatch: " << baStrTargets.size() << " " << nTargets*sizeof(struct QPoi::sTarget);
            m_pOwner->m_simulationTimer.stop();
            return;
        }
    }

    // text file output
    QTextStream tsPeleng(&m_qfPeleng);
    if (!bStarted) {
        m_qfPeleng.resize(0);
        m_qfPeleng.open(QIODevice::ReadWrite);
        if (m_qfPeleng.isOpen()) {
            tsPeleng << QString("StrobNo").rightJustified(10);
            for (int iBeam1=0; iBeam1<QPOI_NUMBER_OF_BEAMS-1; iBeam1++) {
                for (int iBeam2=iBeam1+1; iBeam2<QPOI_NUMBER_OF_BEAMS; iBeam2++) {
                    tsPeleng << "\t" << QString("%1-%2").arg(iBeam1).arg(iBeam2).rightJustified(10);
                }
            }
            tsPeleng << endl;
        }
        bStarted=true;
    }
    if (m_qfPeleng.isOpen()) tsPeleng << QString("%1").arg(iStrob).rightJustified(10);

    // display target markers
    for (int iBeam1=0; iBeam1<QPOI_NUMBER_OF_BEAMS-1; iBeam1++) {
        for (int iBeam2=iBeam1+1; iBeam2<QPOI_NUMBER_OF_BEAMS; iBeam2++) {

            // if beams should not be displayed in QTargetsMap then continue
            if (!m_pTargetsMap->bBeamsUsedForPeleng(iBeam1,iBeam2)) {
                if (m_qfPeleng.isOpen()) tsPeleng << QString("\t%1").arg(QChar('X'),10);
                continue;
            }

            // number of detected targets for each beam
            int nTargets1 = ql_nTargets[iBeam1];
            int nTargets2 = ql_nTargets[iBeam2];

            // qDebug() << iBeam1 << "-" << iBeam2 << ": " << nTargets1 << ", " << nTargets2;
            bool bMatchFound = false;
            for (int iTarget1=0; iTarget1<nTargets1; iTarget1++) {
                for (int iTarget2=0; iTarget2<nTargets2; iTarget2++) {
                    struct QPoi::sTarget *pTarData1 = (struct QPoi::sTarget *)ql_baTarForBeam[iBeam1].data()+iTarget1;
                    struct QPoi::sTarget *pTarData2 = (struct QPoi::sTarget *)ql_baTarForBeam[iBeam2].data()+iTarget2;
                    QPoint qpTar1=pTarData1->qp_rep;
                    QPoint qpTar2=pTarData2->qp_rep;
                    if ((qpTar1-qpTar2).manhattanLength() < QPOI_MAXIMUM_TG_MISMATCH) { // Targets must be close
                        if (bMatchFound) {
                            qDebug() << "Target collision. Strob=" << iStrob << " Beam1=" << iBeam1 << " Beam2=" << iBeam2;
                            continue;
                        }
                        double dM2Tar1=pTarData1->y2mc_rep;
                        double dM2Tar2=pTarData2->y2mc_rep;
                        double dFrac=(dM2Tar1-dM2Tar2)/(dM2Tar1+dM2Tar2);
                        QPointF qpfAvr = (pTarData1->qpf_wei + pTarData2->qpf_wei)/2;
                        // shuffle target markers along velocity axis to avoid hiding one marker under another
                        if (m_bUseShuffle) {
                            qpfAvr.ry()+=NR::ran1(QPoi::m_idum)-0.5e0;
                        }
                        QString qsLegend("%1 %2-%3 F:%4");
                        m_pTargetsMap->addTargetMarker(new QTargetMarker(qpfAvr,
                                qsLegend.arg(iStrob).arg(iBeam1).arg(iBeam2).arg(dFrac,0,'f',2)));
                        if (m_qfPeleng.isOpen()) tsPeleng << QString("\t%1").arg(dFrac,10,'f',2);
                        bMatchFound = true;
                    }
                    else {
                        qDebug() << "qpTar1!=qpTar2: " << qpTar1 << " " << qpTar2;
                    }
                }
            }
            if (!bMatchFound && m_qfPeleng.isOpen()) tsPeleng << QString("\t%1").arg(QChar('-'),10);
        }
    }
    if (m_qfPeleng.isOpen()) tsPeleng << endl;
}

