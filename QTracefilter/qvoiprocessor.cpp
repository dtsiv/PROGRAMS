#include "qvoiprocessor.h"
#include <QDateTime>
#include <QtGlobal>

#include <strstream>

#include "qtracefilter.h"
#include "qproppages.h"
#include "qindicator.h"
#include "rmoexception.h"

#define QVOIPROCESSOR_DUMMY_DEFAULT -9999999.9e99

double QVoiProcessor::m_dFltSigmaS2=QVOIPROCESSOR_DUMMY_DEFAULT;     // Kalman filter: system speed sigma2 m2/s3
double QVoiProcessor::m_dFltSigmaM=QVOIPROCESSOR_DUMMY_DEFAULT;      // Kalman filter: measurement sigma m
double QVoiProcessor::m_dFltHeight=QVOIPROCESSOR_DUMMY_DEFAULT;      // Kalman filter: height km
double QVoiProcessor::m_dIntegrStep=1.0e0;                           // Time integration step s
bool QVoiProcessor::m_bUseGen=true;                                  // use imitator instead of POITE db
double QVoiProcessor::m_dTrajDuration=QVOIPROCESSOR_DUMMY_DEFAULT;   // trajectory duration min
bool QVoiProcessor::m_pbSkipPost[5]={false,false,false,false,false};  // post id to skip
double QVoiProcessor::m_dStrobSpreadSpeed=QVOIPROCESSOR_DUMMY_DEFAULT; // spread speed m/s
double QVoiProcessor::m_dStrobSpreadMin=QVOIPROCESSOR_DUMMY_DEFAULT;   // min spread km
int QVoiProcessor::m_iChi2Prob=0;                                   // chi2 statistics probability threshold
bool QVoiProcessor::m_bUseMatRelax=true;                              // use cov relaxation
double QVoiProcessor::m_dMatRelaxTime=1.0e3;                          // cov relax time

extern QList<double> qlChi2;
/*
m & $\alpha=0.2$ & $\alpha=0.1$ & $\alpha=0.05$ & $\alpha=0.02$ & $\alpha=0.01$ & $\alpha=0.001$ 
1 & 1.642        & 2.706        & 3.841         & 5.412         & 6.635         & 10.827         
2 & 3.219        & 4.605        & 5.991         & 7.824         & 9.210         & 13.815         
3 & 4.642        & 6.251        & 7.815         & 9.837         & 11.341        & 16.268         
*/
double QVoiProcessor::m_dChi2QuantProb[CHI2_NUM_QUANTILES]={
	0.2, 0.1, 0.05, 0.02, 0.01, 0.001
};
double dChi2Quant[4][CHI2_NUM_QUANTILES] ={
// $\alpha=0.2$ & $\alpha=0.1$ & $\alpha=0.05$ & $\alpha=0.02$ & $\alpha=0.01$ & $\alpha=0.001$ 
	{0,             0,             0,              0,              0,              0 },        
	{1.642,         2.706,         3.841,          5.412,          6.635,          10.827},     // m=1    
	{3.219,         4.605,         5.991,          7.824,          9.210,          13.815},     // m=2    
	{4.642,         6.251,         7.815,          9.837,          11.341,         16.268}      // m=3
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QVoiProcessor::QVoiProcessor(QTraceFilter *pOwner, QPoiModel *pPoiModel,QObject *parent/* =0 */) 
: QObject(parent)
, m_pMainCtrl(NULL)
, m_pOwner(pOwner)
, m_pGen(NULL) 
, m_pPoiModel(pPoiModel)
, m_pPm(NULL)
, m_pPp(NULL)
, m_pQ(NULL)
, m_pR(NULL)
, m_pF(NULL)
, m_pH(NULL) 
, m_dMeasNonlinearity(0)
, m_pHnum(NULL) 
, m_pK(NULL)
, m_pZ(NULL)
, m_pZm(NULL)
, m_pXp(NULL)
, m_pXm(NULL)
, m_bQinitialized(false)
, m_bFinitialized(false)
, m_bHinitialized(false)
, m_iTlast (-1) 
, m_bInit(false) 
, m_iPt(0) {
	m_dHei = m_dFltHeight*1.0e3;
	m_iLegendTypePrimary = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_PRIMARY);
	m_iLegendTypeSource = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_SOURCE);
	m_iLegendTypeSourceAlarm = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_SOURCE_ALARM);
	m_iLegendTypeFilter = QIndicator::m_lsLegend.m_qlSettingNames.indexOf(QINDICATOR_LEGEND_FILTER);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QVoiProcessor::~QVoiProcessor() {
	if (m_pGen) delete m_pGen;
	if (m_pPm) delete m_pPm;
	if (m_pPp) delete m_pPp;
	if (m_pQ) delete m_pQ;
	if (m_pR) delete m_pR;
	if (m_pF) delete m_pF;
	if (m_pK) delete m_pK;
	if (m_pH) delete m_pH;
	if (m_pHnum) delete m_pHnum;
	if (m_pZ) delete m_pZ;
	if (m_pZm) delete m_pZm;
	if (m_pXp) delete m_pXp;
	if (m_pXm) delete m_pXm;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QVoiProcessor::init(QString qsMainctrlCfg) {
	if (!m_geoUtils.readCfgFile(qsMainctrlCfg)) return false;
	m_pMainCtrl=m_geoUtils.m_pMainCtrl;
	m_geoUtils.getViewPoint(&m_blhViewPoint,m_pMainCtrl);

	MAINCTRL_P mp=m_pMainCtrl->p;
	if (mp.dwPosCount<5) {
		throw RmoException(QString("QVoiProcessor::mp.dwPosCount = %1").arg(mp.dwPosCount));
		return false;
	}
	m_pGen = new QGenerator(&m_geoUtils,
		QDateTime::fromString("02.06.2017-09.00.22.639",TIMESTAMP_FORMAT).toMSecsSinceEpoch(),
		&m_blhViewPoint);

	// reset seed of random number generators
    m_pGen->resetRandomNumberGenerators();

	m_pPm = new Matrix(4,4);
	m_pPp = new Matrix(4,4);
	m_pQ = new Matrix(4,4);
	m_pF = new Matrix(4,4);
	// posterior state estimate \hat{x}_{k-1(+)}
	m_pXp = new Matrix(4,1);
	// prior state estimate \hat{x}_{k(-)}
	m_pXm = new Matrix(4,1);

    // unity
    //    1.000e+00,	 0.000e+00,	 0.000e+00,	 0.000e+00,	 
    //    0.000e+00,	 1.000e+00,	 0.000e+00,	 0.000e+00,	 
    //    0.000e+00,	 0.000e+00,	 1.000e+00,	 0.000e+00,	 
    //    0.000e+00,	 0.000e+00,	 0.000e+00,	 1.000e+00

	// system noise sigma 10 m/s???
    //    1.041e-06,	 6.980e-02,	-1.870e-07,	-7.978e-03,	 
    //    6.980e-02,	 8.905e+03,	-1.241e-02,	-9.532e+02,	
    //    -1.870e-07,	-1.241e-02,	 3.714e-08,	 2.672e-03,	
    //    -7.978e-03,	-9.532e+02,	 2.672e-03,	 7.710e+02

	// system noise sigma2 1 m2/s3
	// 1.376e-07,	 3.806e-03,	-6.601e-08,	-1.186e-03,	 
	// 3.806e-03,	 2.121e+02,	-1.797e-03,	-5.786e+01,	
	// -6.601e-08,	-1.797e-03,	 3.247e-08,	 6.746e-04,	
	// -1.186e-03,	-5.786e+01,	 6.746e-04,	 4.736e+01

	// 
	// 1.376e-07,	 3.806e-03,	-6.601e-08,	-1.186e-03,	 
	// 3.806e-03,	 2.121e+02,	-1.797e-03,	-5.786e+01,	
	// -6.601e-08,	-1.797e-03,	 3.247e-08,	 6.746e-04,	
	// -1.186e-03,	-5.786e+01,	 6.746e-04,	 4.736e+01

	double dMatrixPeq[]={
        1.000e+00,	 0.000e+00,	 0.000e+00,	 0.000e+00,	 
        0.000e+00,	 1.000e+00,	 0.000e+00,	 0.000e+00,	 
        0.000e+00,	 0.000e+00,	 1.000e+00,	 0.000e+00,	 
        0.000e+00,	 0.000e+00,	 0.000e+00,	 1.000e+00
	};
	Matrix mMatrixPeq(4,4,dMatrixPeq);
	(*m_pPp)=mMatrixPeq;

	// raise init flag
	m_bInit=true;

	return true;
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
void QVoiProcessor::startSimulation(quint64 tFrom, quint64 tTo) {
	QList<int> qlWinIdxs = m_pPoiModel->buildSlidingWindow(tFrom,tTo);
	for (int i=0; i<qlWinIdxs.size(); i++) {
		qint64 iTime = m_pPoiModel->getPoiteTime(qlWinIdxs.at(i));
		TPoiT* pTPoit = m_pPoiModel->getTPoiT(qlWinIdxs.at(i));
		pTPoit->dRMinimum=1.0e-1;
		XYPOINT ptTg = pTPoit->m_pt;
		// qDebug() << "ptTg.dX,ptTg.dY=" << ptTg.dX << " " << ptTg.dY;
		emit indicatorPoint(ptTg.dX,ptTg.dY,m_iLegendTypePrimary,pTPoit);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::startImitator(quint64 iTfrom, quint64 iTto) {
	if (!m_pGen) return;

	// reset imitator time 
	quint64 iTcur=iTfrom;
    m_pGen->resetTime(iTcur);

	// reset seed of random number generators
	m_pGen->resetRandomNumberGenerators();

    // cleanup matrix debug files
	matrixDebugOutput(false);

	// initialize the state vector
	BLH blhTg; // m_dLat, dLon - radians; dHei - meters
	double dVx,dVy; // (meters / sec)
	blhTg = *m_pGen -> getTg();
	m_pGen -> getTgVel(dVx,dVy);

	// posterior state estimate \hat{x}_{k-1(+)}
	Matrix mXp = (*m_pXp);
	mXp[0][0] = blhTg.dLat;
	mXp[1][0] = dVy;
	mXp[2][0] = blhTg.dLon;
	mXp[3][0] = dVx;
    (*m_pXp)= mXp;

	//ostrstream myString;
    //myString << "mXp:" << endl;
	//myString << mXp;
	//qDebug() << QString::fromAscii(myString.str());

	m_iTlast = iTcur;
	m_iTstart= iTcur;
	m_bMatRelaxed=false;
	m_qlErrB.clear(); m_qlErrL.clear();
	m_qlZ[0].clear();m_qlZ[1].clear();m_qlZ[2].clear();
	m_pGen->m_iTstart = m_iTstart;
	while (iTcur < iTto) {
					
		// qDebug() << "Imitator time loop ";

		iTcur += m_pGen->m_dGenDelay*1000.0e0; // msec
		if (!m_pGen->propagate(iTcur)) return;
		int iIdlePostNumber=std::rand()%4+1;

		// DEBUGGING KALMAN FILTER!!!
		// iIdlePostNumber = 2;
		iIdlePostNumber = -1;

		PPOITE pPoite = m_pGen->getPoite(QVoiProcessor::m_pbSkipPost);
		if (!pPoite) { 
		    qDebug() << "QVoiProcessor::pPoite==NULL";
			return;
		}

		// Implement skipped posts
		if (pPoite->Count<1 || pPoite->Count>3) continue;

		// filter step
		filterStep(iTcur,pPoite);

		XYZ xyzFlt; BLH blhFlt;
	    // posterior state estimate \hat{x}_{k(+)}
		Matrix mXp = (*m_pXp);
		blhFlt.dLat=mXp[0][0];
		blhFlt.dLon=mXp[2][0];
		blhFlt.dHei=m_dHei;
		m_geoUtils.toTopocentric(&m_blhViewPoint,&blhFlt,&xyzFlt);
		TPoiT *pTPoitDummy1 = new TPoiT(pPoite);
		qDebug() << "pTPoitDummy1->m_iMatchCount=" << pTPoitDummy1->m_iMatchCount;
		emit indicatorPoint(xyzFlt.dX,xyzFlt.dY,m_iLegendTypeFilter,pTPoitDummy1);

        // topocentric view of filter input (primary points)
		bool bSourceAvailable=true;
		TPoiT *pTPoit = new TPoiT(pPoite);
		pTPoit->dRMinimum=1.0e-1;
		if (pTPoit->CalculateXY()) { // 2D equation
			XYPOINT ptTg = pTPoit->m_pt;
			emit indicatorPoint(ptTg.dX,ptTg.dY,m_iLegendTypePrimary,pTPoit);
		}
		else {
			bSourceAvailable=false;
			// qDebug() << "!pTPoit->CalculateXY()";
		}

        // topocentric view of source
		BLH blhGenSRC=*m_pGen->getTg();
		XYZ xyzGenSRC;
		m_geoUtils.toTopocentric(&m_blhViewPoint,&blhGenSRC,&xyzGenSRC);
		TPoiT *pTPoitDummy2 = new TPoiT(pPoite);
		qDebug() << "pTPoitDummy2->m_iMatchCount=" << pTPoitDummy2->m_iMatchCount;
		if (bSourceAvailable) {
		    emit indicatorPoint(xyzGenSRC.dX,xyzGenSRC.dY,m_iLegendTypeSource,pTPoitDummy2);
		}
		else {
		    emit indicatorPoint(xyzGenSRC.dX,xyzGenSRC.dY,m_iLegendTypeSourceAlarm,pTPoitDummy2);
		}

		//---!!! to be deleted inside qindicator !!!
		//delete pTPoit;
		//delete pTPoitDummy1;
		//delete pTPoitDummy2;
		delete[] pPoite;
	}

	//----------------- build sample distributions ---------------------
	exactStatistics();

	//----------------- update indicator view --------------------------
	emit indicatorUpdate();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::filterStep(qint64 iTk, PPOITE pPoite) {
	m_iPt++;

	//---------------------------- time interval evaluation --------------------------
	if (m_iTlast==-1 || iTk<=m_iTlast) {
		throw RmoException("QVoiProcessor::filterStep m_iTlast==-1 || iTk<=m_iTlast");
		return;
	}
    m_dDeltaT=(iTk-m_iTlast)*1.0e-3; // seconds
	if (m_dDeltaT>10.0e0*60*60 || m_dDeltaT<=0.0e0) {
		throw RmoException(QString("QVoiProcessor::filterStep m_dDeltaT=%1 sec").arg(m_dDeltaT));
		return;
	}

	//---------------------------- auxilary geodetic vars --------------------------
    refreshGeodeticParams();

	//---------------------------- measurement validation --------------------------
	if (!spaceStrob(iTk,pPoite)) {
	    // throw RmoException("Space strob rejected!");
	    return;
	}

	//---------------------------- system force matrix --------------------------
	if (!m_bFinitialized) {
		// m_bFinitialized=true;
        refreshMatrixF();
	}

	//---------------------------- system noise covariation --------------------------
	if (!m_bQinitialized) {
        // m_bQinitialized=true;
        refreshMatrixQ(m_dDeltaT);
		//ostrstream myString;
		//myString << (*m_pQ);
	    //qDebug() << "mQ:" << endl << QString::fromAscii(myString.str());
		//qDebug() << "m_dGeoE=" << m_dGeoE;
		//qDebug() << "m_dGeoG=" << m_dGeoG;
		//qDebug() << "m_dGeoCosB=" << m_dGeoCosB;
		//qDebug() << "m_dGeoSinB=" << m_dGeoSinB;
		//
		//qDebug() << "m_dGeoEPrime=" << m_dGeoEPrime;
		//qDebug() << "m_dGeoE2=" << m_dGeoE2;
		//qDebug() << "m_dKalf1=" << m_dKalf1;
		//qDebug() << "m_dKalf3=" << m_dKalf3;
		//qDebug() << "m_dGeoTgB=" << m_dGeoTgB;
		//qDebug() << "m_dGeoG2=" << m_dGeoG2;
		//qDebug() << "m_dGeoXi3=" << m_dGeoXi3;
		//qDebug() << "m_dGeoEps2=" << m_dGeoEps2;
		//qDebug() << "m_dKalf3=" << m_dKalf3;
	}

	//---------------------------- measurement covariation --------------------------
	refreshMatrixR(pPoite);

	//---------------------------- parse POITE --------------------------
	assign_mZ(pPoite);

	//------------ posprior estimate of state: \hat{x}_{k-1(+)} ----------------
	Matrix mXp = (*m_pXp); 

	//------------ state vector increment ------------
	Matrix mXprimeDt(4,1);
	mXprimeDt[0][0] = m_dDeltaT * m_dKalf1;
	mXprimeDt[1][0] = 0.0e0;
	mXprimeDt[2][0] = m_dDeltaT * m_dKalf3;
	mXprimeDt[3][0] = 0.0e0;

	//------------ a priori estimate of state: \hat{x}_{k(-)} -----------------
	Matrix mXm = (*m_pXm);
	mXm = mXp + mXprimeDt;
    (*m_pXm) = mXm;

    //------------------ measurement sensitivity matrix -----------------------
	if (!m_bHinitialized) {
		// m_bHinitialized=true;
	    refreshMatrixH(pPoite);
	    refreshMatrixHnum(pPoite);
	}

	//--------------- system noise covariation P_{k(-)} evolution ------------------
	if (m_bUseMatRelax && !m_bMatRelaxed) {
        refreshMatrixK(pPoite);
        matrixRelaxation(m_dMatRelaxTime);
        m_bMatRelaxed=true;
	}
    matrixEvolution(m_dDeltaT);
	Matrix mKalQ = (*m_pQ);
	Matrix mKalPm= (*m_pPm);
	mKalPm = mKalPm + mKalQ;
    (*m_pPm)=mKalPm;

	//------------ Kalman gain ----------------
    refreshMatrixK(pPoite);

    //--------------- system noise posterior covariation P_{k(+)} ------------------
	Matrix mKalPp= (*m_pPp);
	Matrix mKalK = (*m_pK);
	Matrix mKalH = (*m_pH);
	Matrix mKalR = (*m_pR);

	//------------ posterior covariations ------------
	double dIdentity[]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
	Matrix mBracket(4,4,dIdentity);
	mBracket= mBracket-mKalK*mKalH;
    mKalPp = mBracket * mKalPm * (~mBracket)
	    + mKalK * mKalR * (~mKalK);
	(*m_pPp)= mKalPp;

	//------------ make the actual filter step ------------
	Matrix mZ    = (*m_pZ);
	// posterior measurement estimate \hat{z}_{k(-)}
	Matrix mZm   = (*m_pZm);
    // mismatch correction
    Matrix mMismatchCorr=mKalK*(mZ-mZm);
    mXp = mXm + mMismatchCorr;

	//------------ state estimate errors ------------------
	PBLH pTgBLH=m_pGen->getTg();
	Matrix mTildeX(4,1);
	Matrix mTildeZ=mZ;
	mTildeX[1][0]=mTildeX[3][0]=0.0e0;
    mTildeX[0][0]=mXp[0][0]-pTgBLH->dLat;
    mTildeX[2][0]=mXp[2][0]-pTgBLH->dLon;
	m_qlErrB.append(mXp[0][0]-pTgBLH->dLat);
	m_qlErrL.append(mXp[2][0]-pTgBLH->dLon);
	if (mTildeZ.rowno()==3) {
		// Matrix mKalHnum=(*m_pHnum);
        mTildeZ=mKalH*mTildeX;
		for (int i=0; i<mTildeZ.rowno(); i++) {
			m_qlZ[i].append(mTildeZ[i][0]);
		}
	}

    //------------ statistics chi-squared -----------------
    Matrix mMismatch = (mZ - mZm) - mKalH*mMismatchCorr;
	Matrix mMismatchCov = mKalH*mKalPp*(~mKalH) + mKalR;
	if (mMismatchCov.det() < QVOIPROCESSOR_EPS) throw RmoException(QString("mMismatchCov.det()=%1").arg(mMismatchCov.det()));
	Matrix mStatChi2 = (~mMismatch)*(!mMismatchCov)*mMismatch;

	int nDim=qBound(1,(int)mZ.colsize(),3);
	if (m_iChi2Prob>=sizeof(dChi2Quant[nDim])/sizeof(dChi2Quant[nDim][0])) {
		throw RmoException("Illegal m_iChi2Prob");
	}
	double dChi2Thresh=dChi2Quant[nDim][m_iChi2Prob];
	if (mStatChi2[0][0]>dChi2Thresh) {
		qDebug() << m_iPt << " Chi2 statistics " << mStatChi2[0][0] << "  over thresh " << dChi2Thresh;
	}
    qlChi2 << mStatChi2[0][0];
	if (0) {
		Matrix mKalF=(*m_pF);
		Matrix mKalQ=(*m_pQ);
		Matrix mKalH=(*m_pH);
		Matrix mKalR=(*m_pR);

		Matrix mKalPp=(*m_pPp);
		Matrix mKalPm(4,4);

		ostrstream myString;

		myString << "mXm\n" << mXm << endl;
		myString << "m_dDeltaT\n" << m_dDeltaT << endl;
		myString << "mXprimeDt\n" << mXprimeDt << endl;
		myString << "mKalR\n" << mKalR << endl;
		myString << "mKalQ\n" << mKalQ << endl;
		myString << "(*m_pPm)\n " << (*m_pPm) << endl;
		myString << "(*m_pPp)\n " << (*m_pPp) << endl;
		myString << "mKalH\n" << mKalH << endl;
		if (m_pHnum) {
			Matrix mKalHnum=(*m_pHnum);
			myString << "mKalHnum\n" << mKalHnum << endl;
		}
        myString << "MeasNonlinearity\n" << m_dMeasNonlinearity << endl;
		myString << "mKalK\n" << mKalK << endl;
		//myString << "mKalH*mXprimeDt\n" << mKalH*mXprimeDt << endl;
		myString << "mKalK*mKalH\n" << mKalK*mKalH << endl;
		//myString << "mBracket\n" << mBracket << endl;
		//myString << "m_dDeltaT * mKalF \n " << (m_dDeltaT * mKalF) << endl;
		myString << "mZ\n" << mZ << endl;
		myString << "mZm\n" << mZm << endl;
		myString << "mZ-mZm\n" << mZ-mZm << endl;
		myString << "mKalK*(mZ-mZm)\n" << mKalK*(mZ-mZm) << endl;
		myString << "\0\0\0\0\0";

	    qDebug() << "==================================================================";
        qDebug() << QString::fromLocal8Bit(myString.str());
	}

	//------------ save the posterior state estimate \hat{x}_{k(+)} ------------
	(*m_pXp) = mXp;

	//------------------- write matrices to text files -----------------------
	matrixDebugOutput(true,iTk);
    covEigenvectors(iTk);

	//------------------- update current system time  -----------------------
	m_iTlast=iTk;

//------------------
	if (mKalH.rowno()==3) {
        int i,j,k;
		const int NP=3;
		const DP TINY=1.0e-6;
		Vec_DP d(NP),e(NP),f(NP);
		Mat_DP a(NP,NP),c(NP,NP);
		Matrix mCov=mKalH*mKalPp*(~mKalH);
		for(i=0;i<NP;i++) for(j=0;j<NP;j++) {
			a[i][j]=mCov[i][j];
		}
		NR::tred2(a,d,e);
		NR::tqli(d,e,a);
		for (i=0;i<NP-1;i++) {
			for (j=i+1;j<NP;j++) {
				if (d[i]>d[j]) {
					double dTmp;
					dTmp=d[i]; d[i]=d[j]; d[j]=dTmp;
					for (k=0;k<NP;k++) {
						dTmp=a[k][i]; a[k][i]=a[k][j]; a[k][j]=dTmp;
					}
				}
			}
		}
		//qDebug() << "HP~H eigenval " << d[0] << " " << d[1] << " " << d[2];
		//qDebug() << "evec[0] " << a[0][0] << " " << a[1][0] << " " << a[2][0];
		//qDebug() << "evec[1] " << a[0][1] << " " << a[1][1] << " " << a[2][1];
		//qDebug() << "evec[2] " << a[0][2] << " " << a[1][2] << " " << a[2][2];
	}

	//------------------- cleanup -------------------------------------------
	if (m_pZ) {
		delete m_pZ;
		m_pZ = NULL;
	}
	if (m_pZm) {
		delete m_pZm;
		m_pZm = NULL;
	}
	if (m_pK) {
		delete m_pK;
		m_pK = NULL;
	}
	if (m_pH) {
		delete m_pH;
		m_pH = NULL;
	}
	if (m_pHnum) {
		delete m_pHnum;
		m_pHnum = NULL;
	}
	if (m_pR) {
		delete m_pR;
		m_pR = NULL;
	}
}
