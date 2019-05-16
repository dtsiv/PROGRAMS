#include "qvoiprocessor.h"
#include <QDateTime>
#include <QtGlobal>

#include <strstream>

#include "qtracefilter.h"
#include "qproppages.h"
#include "qindicator.h"
#include "rmoexception.h"

#define QVOIPROCESSOR_DUMMY_DEFAULT -9999999.9e99

bool QVoiProcessor::m_bUseGen=true;                                  // use imitator instead of POITE db
double QVoiProcessor::m_dTrajDuration=QVOIPROCESSOR_DUMMY_DEFAULT;   // trajectory duration min
bool QVoiProcessor::m_pbSkipPost[5]={false,false,false,false,false};  // post id to skip
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QVoiProcessor::QVoiProcessor(QTraceFilter *pOwner, QPoiModel *pPoiModel,QObject *parent/* =0 */) 
: QObject(parent)
, m_pMainCtrl(NULL)
, m_pOwner(pOwner)
, m_pGen(NULL) 
, m_pPoiModel(pPoiModel)
, m_bInit(false) 
      {
	m_iLegendTypePrimary = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_PRIMARY);
	m_iLegendTypeSource = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_SOURCE);
	m_iLegendTypeSourceAlarm = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_SOURCE_ALARM);
	m_iLegendTypeFilter = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_FILTER);
    m_iLegendTypeCluster = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_CLUSTER);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QVoiProcessor::~QVoiProcessor() {
	if (m_pGen) delete m_pGen;
    while (m_qlKalmanFilters.size()) delete m_qlKalmanFilters.takeLast();
    while (m_qlClusters.size()) delete m_qlClusters.takeLast();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::start(quint64 iTfrom, quint64 iTto, bool bUseGen) {

    if (bUseGen) { // fabricate POITE from QGenerator
        // running time
        quint64 iTcur=iTfrom;

        // initialize POITE generator
        if (!m_pGen) return;
        m_pGen->resetTime(iTcur);
        m_pGen->resetRandomNumberGenerators();
        m_pGen->m_iTstart = m_iTstart= iTcur;

        //=================== prepare next PPOITE pPoite ======================
        iTcur += m_pGen->m_dGenDelay*1000.0e0; // msec
        if (!m_pGen->propagate(iTcur)) return;

        while (iTcur < iTto) {
            iTcur += m_pGen->m_dGenDelay*1000.0e0; // msec
            if (!m_pGen->propagate(iTcur)) return;
            PPOITE pPoite = m_pGen->getPoite(QVoiProcessor::m_pbSkipPost);
            PBLH pblhGenSRC=m_pGen->getTg();
            receiveCodogram (iTcur,pPoite,pblhGenSRC,-1);
        }
    }
    else { // obtain experimental POITE from data base
        for (int iRawIdx=0; iRawIdx<m_pPoiModel->getRawListSize(); iRawIdx++) {
            qint64 iTime = m_pPoiModel->getPoiteTime(iRawIdx);
            if (iTime<iTto && iTime>=iTfrom) {
                // if (m_qlClusters.size()>10) break;
                QByteArray baPoite = m_pPoiModel->getPPoite(iRawIdx);
                if (baPoite.isEmpty()) continue;
                PPOITE pPoite = (PPOITE) new char[baPoite.size()];
                std::memcpy(pPoite,baPoite.data(),baPoite.size());
                PBLH pblhGenSRC=NULL;
                receiveCodogram (iTime,pPoite,pblhGenSRC,iRawIdx);
            }
        }
    }

    //----------------- update indicator view --------------------------
    emit indicatorUpdate();
    while (m_qlKalmanFilters.size()) delete m_qlKalmanFilters.takeLast();
    while (m_qlClusters.size()) delete m_qlClusters.takeLast();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::receiveCodogram(quint64 iTcur, PPOITE pPoite, BLH *pblhGenSRC /* =NULL */, int iRawIdx /* =-1 */) {

    // cluster cutoff radius (m)
    double dRcutoff=QKalmanFilter::m_dClusterCutoff*1.0e3;

    // initialize flag: 2D hyperbolic equation has solution (primary source point)
    bool bPrimaryPointAvailable=true;
    XYPOINT ptTg;
    if (pPoite) {
        if (pPoite->Count<1 || pPoite->Count>3) { delete [] pPoite; return;}

        // topocentric view of noisy source
        TPoiT *pTPoiT = new TPoiT(pPoite);
        pTPoiT->dRMinimum=1.0e-1;
        if (QPsVoi::m_UseTolerance) {
            if (pTPoiT->CalculateXY()) {
                ptTg = pTPoiT->m_pt;
                emit indicatorPoint(ptTg.dX,ptTg.dY,m_iLegendTypePrimary,pTPoiT,iRawIdx);
            }
            else {
                bPrimaryPointAvailable=false;
                delete pTPoiT; // pTPoit was not passed to new owner QIndicator
            }
        }
        else { // improved 2D solver
            if (pTPoiT->CalculateXY_baseSelection()) {
                pTPoiT->m_pt.dX = pTPoiT->m_x[0];
                pTPoiT->m_pt.dY = pTPoiT->m_y[0];
                ptTg = pTPoiT->m_pt;
                emit indicatorPoint(ptTg.dX,ptTg.dY,m_iLegendTypePrimary,pTPoiT,iRawIdx);
            }
            else {
                bPrimaryPointAvailable=false;
                delete pTPoiT; // pTPoit was not passed to new owner QIndicator
            }
        }
        if (qIsNaN(ptTg.dX) || qIsNaN(ptTg.dY)) {
            throw RmoException("receiveCodogram: qIsNan(ptTg.dX) || qIsNan(ptTg.dY)");
            return;
        }

        // topocentric view of exact tg position
        if (pblhGenSRC) {
            BLH blhGenSRC=*pblhGenSRC;    // m_dLat, dLon - radians; dHei - meters
            XYZ xyzGenSRC;                // topocentric coordinates (meters)
            m_geoUtils.toTopocentric(&m_blhViewPoint,&blhGenSRC,&xyzGenSRC);
            TPoiT *pTPoitDummy = new TPoiT(pPoite);
            if (!pTPoitDummy) throw RmoException("QVoiProcessor: new TPoiT(pPoite)==NULL");
            if (bPrimaryPointAvailable) {
                emit indicatorPoint(xyzGenSRC.dX,xyzGenSRC.dY,m_iLegendTypeSource,pTPoitDummy,-1);
            }
            else {
                emit indicatorPoint(xyzGenSRC.dX,xyzGenSRC.dY,m_iLegendTypeSourceAlarm,pTPoitDummy,-1);
            }
        }
    }
    else throw RmoException("QVoiProcessor::receiveCodogram pPoite is NULL");

    //=================== PPOITE pPoite is now ready for processing ======================
    // test POITE data to fit an existing trajectory filter
    if (m_qlKalmanFilters.size()) {
        QList<double> qlLatOffsets; // primary point distances from the existing trajectories
        QList<int> qlIdx; // index array
        // list all lateral offsets
        for (int i=0; i<m_qlKalmanFilters.size(); i++) {
            QKalmanFilter *pKalmanFilter=m_qlKalmanFilters.at(i);
            // dMaxLatOffset will be used for concurrent trajectory selection
            double dMaxLatOffset;
            bool bTimeout=false;
            if (pKalmanFilter->spaceStrob(iTcur,pPoite,dMaxLatOffset,bTimeout)) {
                qlLatOffsets.append(dMaxLatOffset);
                qlIdx.append(i);
            }
            else if (bTimeout) {
                delete m_qlKalmanFilters.takeAt(i);
            }
            if (i>=m_qlKalmanFilters.size()) break;
        }
        // sort lateral offsets ascending
        int nFilters=qlIdx.size(); // trajectories near target
        if (nFilters) { // there are filters for tg those pass space strob
            for (int i=0; i<nFilters-1; i++) { // sort clusters from closest to farest
                for (int j=i+1; j<nFilters; j++) {
                    if (qlLatOffsets.at(i)>qlLatOffsets.at(j)) {
                        qlLatOffsets.swap(i,j);
                        qlIdx.swap(i,j);
                    }
                }
            }
            //------------- closest trajectory: filterStep() ---------------
            int iIdxNearest=qlIdx.at(0); // closest trajectory
            if (iIdxNearest>=m_qlKalmanFilters.size()) throw RmoException("QVoiProcessor: iIdxNearest>=m_qlKalmanFilters.size()");
            QKalmanFilter *pKalmanFilter=m_qlKalmanFilters.at(iIdxNearest);
            // perform filter step using new POITE data and issue m_iLegendTypeFilter indicatorPoint
            bool bOverrun=false;
            bool bTimeout=false;
            if (pKalmanFilter->filterStep(iTcur,pPoite,bOverrun,bTimeout)) { // POITE accepted
                XYZ xyzFlt=pKalmanFilter->getTgXYZ();
                TPoiT *pTPoitDummy = new TPoiT(pPoite);
                emit indicatorPoint(xyzFlt.dX,xyzFlt.dY,m_iLegendTypeFilter,pTPoitDummy,-1);
            }
            else { // POITE rejected => kill filter
 //               qDebug() << QString("Killing trajectory: overrun=%1 timeout=%2").arg(bOverrun).arg(bTimeout);
                delete m_qlKalmanFilters.takeAt(iIdxNearest);
            }
            delete [] pPoite; return; // POITE processed
        }
        // if there no passend filters => proceed with clustering
    }

    // POITE data did not match any existing trajectory => add to either existing or new cluster
    if (!bPrimaryPointAvailable) { // if source point is not available - we cannot do clustering
        delete [] pPoite; return;
    }

    // Source point obtained. starting clustering
    double dTgX=ptTg.dX;
    double dTgY=ptTg.dY;
//    qDebug() << QString("Got src pt $(%1,%2). nClusters=%3 nFilters=%4")
//                .arg(dTgX).arg(dTgY)
//                .arg(m_qlClusters.size())
//                .arg(m_qlKalmanFilters.size());
    // check existing clusters for timeout
    for (int i=0; i<m_qlClusters.size(); i++) {
        if (m_qlClusters.at(i)->isStale(iTcur)) {
//            qDebug() << "Deleting stale cluster: " << i;
            delete m_qlClusters.takeAt(i);
        }
        if (i>=m_qlClusters.size()) break;
    }

    // search for closest cluster
    QList<double> qlDist; // distances from the primary point
    QList<int> qlIdx; // index array
    for (int i=0; i<m_qlClusters.size(); i++) {
        double rr=m_qlClusters.at(i)->distance(dTgX,dTgY);
        if (rr > dRcutoff) {
            // qDebug() << "rr=" << rr << " >dRcutoff=" << dRcutoff << " skipping...";
            continue;
        }
        qlDist.append(rr);
        qlIdx.append(i);
    }
    //------------- New cluster creation ---------------
    int nClusters=qlIdx.size(); // clusters near target
    if (nClusters==0) { // no primary points cluster found => create new one
        m_qlClusters.append(new QPrimaryCluster(iTcur,dTgX,dTgY));
        delete [] pPoite; return; // no cluster found near target, new cluster created
    }
    //------------- Nearest cluster: update ---------------
    for (int i=0; i<nClusters-1; i++) { // sort clusters from closest to farest
        for (int j=i+1; j<nClusters; j++) {
            if (qlDist.at(i)>qlDist.at(j)) {
                qlDist.swap(i,j);
                qlIdx.swap(i,j);
            }
        }
    }
    int iIdxNearest=qlIdx.at(0); // closest cluster
    // update and check current size
    if (m_qlClusters.at(iIdxNearest)->appendPriPt(iTcur,dTgX,dTgY)) { // cluster is big enough
        // create Kalman filter
        // target position
        double dS = _hypot(ptTg.dX, ptTg.dY);
        double dA = atan2(ptTg.dX,ptTg.dY);
        if(dA > 2. * M_PI) dA -= 2. * M_PI;
        else if(dA < 0.) dA += 2. * M_PI;
        double dLat,dLon;
        TdCord::Dirct2(m_blhViewPoint.dLon, m_blhViewPoint.dLat, dA, dS, &dLat, &dLon, NULL);
        BLH blhTg; // dLat, dLon (rad); dHei (meters)
        blhTg.dLat=dLat; blhTg.dLon=dLon;
        blhTg.dHei=0; // unused
        double dVx=0.0e0,dVy=0.0e0;
        if (QKalmanFilter::m_bEstIniVelocity) m_qlClusters.at(iIdxNearest)->getVelocityEst(dVx,dVy);
        QKalmanFilter *pKalmanFilter=new QKalmanFilter(iTcur,m_geoUtils,blhTg,dVx,dVy);
        if (!pKalmanFilter) throw RmoException("QVoiProcessor: new QKalmanFilter==NULL");
//        qDebug() << QString("new flt @(%1, %2) vel (%3, %4)")
//                    .arg(ptTg.dX).arg(ptTg.dY)
//                    .arg(dVx).arg(dVy);
        TPoiT *pTPoitDummy = new TPoiT(pPoite);
        if (!pTPoitDummy) throw RmoException("QVoiProcessor: new TPoiT(pPoite)==NULL");
        emit indicatorPoint(ptTg.dX, ptTg.dY,m_iLegendTypeFilter,pTPoitDummy,-1);
        TPoiT *pTPoitDummy1 = new TPoiT(pPoite);
        if (!pTPoitDummy1) throw RmoException("QVoiProcessor: new TPoiT(pPoite)==NULL");
        XYPOINT ptClusterCntr=m_qlClusters.at(iIdxNearest)->massCenter();
        emit indicatorPoint(ptClusterCntr.dX,ptClusterCntr.dY,m_iLegendTypeCluster,pTPoitDummy1,-1);
        m_qlKalmanFilters.append(pKalmanFilter);
        delete m_qlClusters.takeAt(iIdxNearest);
    }
    delete [] pPoite;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QList<int> QVoiProcessor::getActivePosts(PPOITE pPoite) {
    QList<int> qlRetVal;
    for (int i=0; i < pPoite -> Count; i++) {
        qlRetVal.append(pPoite -> rx[i].uMinuIndx-1);
        qlRetVal.append(pPoite -> rx[i].uSubtIndx-1);
    }
    return qlRetVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::listPosts() {
    if (!m_bInit) return;
    if (!m_pMainCtrl) return;
    MAINCTRL_P mp=m_pMainCtrl->p;
    if (mp.dwPosCount<5) {
        throw RmoException(QString("QVoiProcessor::mp.dwPosCount = %1").arg(mp.dwPosCount));
        return;
    }
    for (int i=1; i<=4; i++) {
        BLH blhPost = mp.positions[i].blh;
        blhPost.dLat*=DEG_TO_RAD;
        blhPost.dLon*=DEG_TO_RAD;
        XYZ xyzPost;
        m_geoUtils.toTopocentric(&m_blhViewPoint,&blhPost,&xyzPost);
        emit addPost(xyzPost.dX,xyzPost.dY,i);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QVoiProcessor::init(QString qsMainctrlCfg) {

    // init m_geoUtils
    if (!m_geoUtils.readCfgFile(qsMainctrlCfg)) return false;
    m_pMainCtrl=m_geoUtils.m_pMainCtrl;
    m_geoUtils.getViewPoint(&m_blhViewPoint,m_pMainCtrl);
    MAINCTRL_P mp=m_pMainCtrl->p;
    if (mp.dwPosCount<5) {
        throw RmoException(QString("QVoiProcessor::mp.dwPosCount = %1").arg(mp.dwPosCount));
        return false;
    }

    // init random numbers generator
    m_pGen = new QGenerator(&m_geoUtils,
        QDateTime::fromString("02.06.2017-09.00.22.639",TIMESTAMP_FORMAT).toMSecsSinceEpoch(),
        &m_blhViewPoint);

    // reset seed of random number generators
    m_pGen->resetRandomNumberGenerators();

    // raise init flag
    m_bInit=true;

    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPrimaryCluster::QPrimaryCluster(qint64 iTk, double dTgX, double dTgY)
      : m_iTimeout(QKalmanFilter::m_dTrajTimeout*60000)
      , m_iStart(iTk) {
    int iClusterMinSize = QKalmanFilter::m_iClusterMinSize;
    if (QVoiProcessor::m_bUseGen) iClusterMinSize = QGenerator::m_iClusterMinSize;
    if ((iClusterMinSize)<2 || (iClusterMinSize)>1000) throw RmoException("QPrimaryCluster: bad m_iClusterMinSize");
    m_qlTk.append(iTk);
    m_qlTgX.append(dTgX);
    m_qlTgY.append(dTgY);
    m_iSize=1;
    m_dCenterX=dTgX;
    m_dCenterY=dTgY;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPrimaryCluster::isStale(qint64 iTk) {
    if (iTk-m_iStart<0) throw RmoException(QString("QPrimaryCluster: iTk (%1) < m_iStart (%2). Diff=%3").arg(iTk).arg(m_iStart).arg(m_iStart-iTk));
    // qDebug() << QString("iTk(%1) > m_iStart(%2)+m_iTimeout(%3)").arg(iTk).arg(m_iStart).arg(m_iTimeout);
    return (iTk > m_iStart+m_iTimeout);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPrimaryCluster::appendPriPt(qint64 iTk, double dTgX, double dTgY) {
    m_qlTk.append(iTk);
    m_qlTgX.append(dTgX);
    m_qlTgY.append(dTgY);
    m_dCenterX=m_dCenterX*m_iSize+dTgX;
    m_dCenterY=m_dCenterY*m_iSize+dTgY;
    m_iSize++;
    m_dCenterX/=m_iSize;
    m_dCenterY/=m_iSize;
    if (m_qlTk.size() != m_iSize || m_qlTgX.size() != m_iSize || m_qlTgY.size() != m_iSize) throw RmoException("QPrimaryCluster size mismatch");
    int iClusterMinSize = QKalmanFilter::m_iClusterMinSize;
    if (QVoiProcessor::m_bUseGen) iClusterMinSize = QGenerator::m_iClusterMinSize;
    return (m_iSize>=iClusterMinSize);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double QPrimaryCluster::distance(double dTgX, double dTgY) {
    double xx=m_dCenterX-dTgX; xx=xx*xx;
    double yy=m_dCenterY-dTgY; yy=yy*yy;
    return sqrt(xx+yy);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPrimaryCluster::getVelocityEst(double &dVx,double &dVy) {
    if (m_iSize<2) throw RmoException("QPrimaryCluster: m_iSize<2");

    double dXT=0.0e0;
    double dX=0.0e0;
    double dYT=0.0e0;
    double dY=0.0e0;
    double dT=0.0e0;
    double dT2=0.0e0;

    for (int i=0; i<m_iSize; i++) {
        double dTimeSecs = (m_qlTk.at(i)-m_qlTk.at(0))*1.0e-3;
        dXT+=m_qlTgX.at(i)*dTimeSecs;
        dYT+=m_qlTgY.at(i)*dTimeSecs;
        dX+=m_qlTgX.at(i);
        dY+=m_qlTgY.at(i);
        dT+=dTimeSecs;
        dT2+=dTimeSecs*dTimeSecs;
    }
    double dDenom=m_iSize*dT2-dT*dT;
    if (abs(dDenom)<QKALMANFILTER_EPS) {
        throw RmoException(QString("QPrimaryCluster: dDenom<QKALMANFILTER_EPS: m_iSize=%1 dT2=%2 dT=%3")
                           .arg(m_iSize).arg(dT2).arg(dT));
    }
    // output velocity estimate (m/s)
    dVx = (m_iSize*dXT-dX*dT)/dDenom;
    dVy = (m_iSize*dYT-dY*dT)/dDenom;
}
