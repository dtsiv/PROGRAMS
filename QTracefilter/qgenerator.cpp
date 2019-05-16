#include "qgenerator.h"
#include "qproppages.h"
#include "rmoexception.h"

double QGenerator::m_dGenX0=QGENERATOR_DUMMY_DEFAULT; // gen init x0
double QGenerator::m_dGenY0=QGENERATOR_DUMMY_DEFAULT; // gen init y0
double QGenerator::m_dGenV0=QGENERATOR_DUMMY_DEFAULT; // gen init v
double QGenerator::m_dGenAz=QGENERATOR_DUMMY_DEFAULT; // gen init Az
double QGenerator::m_dGenTrajR=QGENERATOR_DUMMY_DEFAULT; // gen traj R
double QGenerator::m_bGenTrajCW=QGENERATOR_DUMMY_DEFAULT; // gen traj CW
double QGenerator::m_dGenSigmaM=QGENERATOR_DUMMY_DEFAULT;   // Poite generator: measurement sigma m
double QGenerator::m_dGenDelay=QGENERATOR_DUMMY_DEFAULT;   // Poite generator: measurement delay s
double QGenerator::m_dGenHeight=QGENERATOR_DUMMY_DEFAULT;   // Poite generator: height km
double QGenerator::m_dGenKinkTime=QGENERATOR_DUMMY_DEFAULT;   // Poite generator: kink time min
double QGenerator::m_dGenKinkAngle=QGENERATOR_DUMMY_DEFAULT;   // Poite generator: kink angle deg
int QGenerator::m_iClusterMinSize=0;                            // minimum cluster size to start traj
QGenerator::TrajectoryTypes QGenerator::m_ttGenTrajectory=(QGenerator::TrajectoryTypes)0; // traj type

// Gaussian random number seed
int QGenerator::m_idum=-13;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGenerator::QGenerator(QGeoUtils *pGeoUtils,qint64 iTime,PBLH pblhViewPoint) // RADIANS!!!
 :m_pGeoUtils(pGeoUtils) 
, m_iTime(iTime) 
, m_bKinkPassed(false) {

	double dMajor = m_pGeoUtils->DEFMAJOR;
	double dFlat = (dMajor - m_pGeoUtils->DEFMINOR) / dMajor;
	m_dMajor = dMajor;
    m_dESq = dFlat * (2. - dFlat);

	// view pt
	m_blhViewPoint = *pblhViewPoint;

	// target velocity
	if (m_ttGenTrajectory==LinearTrajectory || m_ttGenTrajectory==LinearWithKink) {
		double dVelVal=m_dGenV0;
		double dVelAz=m_dGenAz*DEG_TO_RAD;
		double dVelAngle = M_PI/2.0e0 - dVelAz; // radians from Ox CCW
		m_dW1 = dVelVal*cos(dVelAngle); // East vel m/s
		m_dW2 = dVelVal*sin(dVelAngle); // North vel m/s
		// target position
		double dTgX=m_dGenX0*1.0e3;
		double dTgY=m_dGenY0*1.0e3;
		double dS = _hypot(dTgX, dTgY);
		double dA = atan2(dTgX,dTgY);
		if(dA > 2. * M_PI) dA -= 2. * M_PI;
		else if(dA < 0.) dA += 2. * M_PI;
		double dLat,dLon;
		TdCord::Dirct2(m_blhViewPoint.dLon, m_blhViewPoint.dLat, dA, dS, &dLat, &dLon, NULL);
		m_blhTg.dLat=dLat; m_blhTg.dLon=dLon;
		setTargetHeight(m_dGenHeight*1.0e3);
	}
	else if (m_ttGenTrajectory==CircleTrajectory) {
		double dVelVal=m_dGenV0;
		double dVelAz=(m_bGenTrajCW?(M_PI/2):(-M_PI/2));
		double dVelAngle = M_PI/2.0e0 - dVelAz; // radians from Ox CCW
		m_dTrajR = m_dGenTrajR*1.0e3;
		m_dW1 = dVelVal*cos(dVelAngle); // East vel m/s
		m_dW2 = dVelVal*sin(dVelAngle); // North vel m/s
		// target position
		double dTgX=0.0e0;
		double dTgY=m_dTrajR;
		double dS = _hypot(dTgX, dTgY);
		double dA = atan2(dTgX, dTgY);
		if(dA > 2. * M_PI) dA -= 2. * M_PI;
		else if(dA < 0.) dA += 2. * M_PI;
		double dLat,dLon;
		TdCord::Dirct2(m_blhViewPoint.dLon, m_blhViewPoint.dLat, dA, dS, &dLat, &dLon, NULL);
		m_blhTg.dLat=dLat; m_blhTg.dLon=dLon;
		setTargetHeight(m_dGenHeight*1.0e3);
	}
	else {
		throw RmoException("Illegal traj type");
	}

	// start time msecs since Unix epoch
	m_iTstart = m_iTime;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGenerator::~QGenerator() { }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGenerator::resetRandomNumberGenerators() {
    // initialize seed of stdlib.h pseudo-random numbers (iSeed is read-only)
	unsigned int iSeed=1;
    std::srand(iSeed);

	// initialize seed of Numerical Recipes Gaussian random numbers (m_idum is read-write)
	this->m_idum=-1;
	NR::gasdev(this->m_idum);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGenerator::setTargetHeight(double dHei) { // meters
#ifdef QGENERATOR_USE_HEIGHT_CORRECTION
	BLH blhTg; // dLat, dLon - radians; dHei - meters
	XYZ xyzTg; // topocentric xyz
	blhTg.dLat=m_blhTg.dLat; blhTg.dLon=m_blhTg.dLon; blhTg.dHei=0.0e0;
	m_pGeoUtils->toTopocentric(&m_blhViewPoint,&blhTg,&xyzTg);
	m_blhTg.dHei = dHei - xyzTg.dZ;
	// qDebug() << " setTargetHeight dHei=" << dHei << " m_blhTg.dHei=" << m_blhTg.dHei;
#else
    m_blhTg.dHei = dHei;
#endif
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PBLH QGenerator::getTg() { 
	return &m_blhTg;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGenerator::getTgVel(double &vx, double &vy) { 
	vx = m_dW1;
	vy = m_dW2;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QGenerator::propagate(qint64 iTime) {
    double dt=1.0e-1; // integration step

	if (m_iTime >= iTime) return false;

	// position-dependant staff
	double dLat = m_blhTg.dLat;
	double dLon = m_blhTg.dLon;
	double dHei = m_blhTg.dHei;

#ifdef QGENERATOR_USE_HEIGHT_CORRECTION
	BLH blhTg; // dLat, dLon - radians; dHei - meters
	XYZ xyzTgBefore; // topocentric xyz
	XYZ xyzTgAfter; // topocentric xyz
	blhTg.dLat=dLat; blhTg.dLon=dLon; blhTg.dHei=m_blhTg.dHei;
	m_pGeoUtils->toTopocentric(&m_blhViewPoint,&blhTg,&xyzTgBefore);
#endif

	double dCurT=0.0e0;  //sec
	while (dCurT < (iTime-m_iTime)*1.0e-3) {
		double cosB = cos(dLat);   double sinB = sin(dLat);
		double cosL = cos(dLon);   double sinL = sin(dLon);
		double ksi = 1.0e0/sqrt(1. - m_dESq * sinB * sinB);
		double dE = (1. - m_dESq)*m_dMajor*ksi*ksi*ksi+dHei;
		double dG = m_dMajor*ksi + dHei;

		// var conversion
		double dAbsTime=(m_iTime - m_iTstart)+dCurT*1.0e3;

		// kink
		double dKinkTime=m_dGenKinkTime*60.0e3;
		if (m_ttGenTrajectory==LinearWithKink && dAbsTime > dKinkTime && !m_bKinkPassed) {
			m_bKinkPassed=true;
			double dKAngle=m_dGenKinkAngle*DEG_TO_RAD;
			dKAngle=(m_bGenTrajCW?(-dKAngle):dKAngle);
			double dW1=m_dW1*cos(dKAngle)-m_dW2*sin(dKAngle);
			double dW2=m_dW1*sin(dKAngle)+m_dW2*cos(dKAngle);
			m_dW1=dW1;
			m_dW2=dW2;
		}

		// integration step
		if (m_ttGenTrajectory==LinearTrajectory || m_ttGenTrajectory==LinearWithKink) {
			double dLatInc=m_dW2/dE*dt;
			double dLonInc=m_dW1/dG/cosB*dt;
			dLat+=dLatInc;
			dLon+=dLonInc;
		}
		else if (m_ttGenTrajectory==CircleTrajectory) {
			double dAngle=m_dGenV0*dAbsTime*1.0e-3/m_dTrajR;
			dAngle=M_PI/2-(m_bGenTrajCW?dAngle:(-dAngle));
			double dAngleV=dAngle+(m_bGenTrajCW?M_PI/2:(-M_PI/2));
			m_dW1=m_dGenV0*cos(dAngleV);
			m_dW2=m_dGenV0*sin(dAngleV);
			// target position
			double dTgX=m_dTrajR*cos(dAngle);
			double dTgY=m_dTrajR*sin(dAngle);
			double dS = _hypot(dTgX, dTgY);
			double dA = atan2(dTgX, dTgY);
			if(dA > 2. * M_PI) dA -= 2. * M_PI;
			else if(dA < 0.) dA += 2. * M_PI;
			TdCord::Dirct2(m_blhViewPoint.dLon, m_blhViewPoint.dLat, dA, dS, &dLat, &dLon, NULL);
			setTargetHeight(m_dGenHeight*1.0e3);
		}
		else {
			throw RmoException("Unknown trajectory type");
		}

		// time increment
		dCurT+=dt;
	}

#ifdef QGENERATOR_USE_HEIGHT_CORRECTION
	// recalculate target height
	blhTg.dLat=dLat; blhTg.dLon=dLon; blhTg.dHei=m_blhTg.dHei;
	m_pGeoUtils->toTopocentric(&m_blhViewPoint,&blhTg,&xyzTgAfter);
	m_blhTg.dHei+=xyzTgBefore.dZ-xyzTgAfter.dZ;
	// qDebug() << " Generator: " << m_blhTg.dHei;
#endif

	// advance QGenerator time (msecs since Unix epoch)
	m_iTime += dCurT*1.0e3;

	// assign the result
	m_blhTg.dLat=dLat;
	m_blhTg.dLon=dLon;
	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PPOITE QGenerator::getPoite(bool bIdlePosts[]) {
	PMAINCTRL pCtrl = m_pGeoUtils->m_pMainCtrl;
	int iActiveBases[5], iActNum=0;
	for (int iBase=0; iBase< sizeof(pCtrl->p.bases)/sizeof(pCtrl->p.bases[0]); iBase++) {
		int iMinuIdx=pCtrl->p.bases[iBase].bMinuIndx;
		int iSubtIdx=pCtrl->p.bases[iBase].bSubtIndx;
		if (!bIdlePosts[iMinuIdx] && !bIdlePosts[iSubtIdx] && iActNum<3) { // maximum 3 bases possible
			iActiveBases[iActNum++]=iBase;
		}
	}
	if (iActNum > 3) {
		throw RmoException(QString("QGenerator::iActNum = %1").arg(iActNum));
		return NULL;
	}
    // qDebug() << "iIdlePostNumber=" << iIdlePostNumber
	//	<< " iMinuIdx,iSubtIdx[0,1] = " 
	//	<< " " << pCtrl->p.bases[iActiveBases[0]].bMinuIndx << " " << pCtrl->p.bases[iActiveBases[0]].bSubtIndx
	//	<< " " << pCtrl->p.bases[iActiveBases[1]].bMinuIndx << " " << pCtrl->p.bases[iActiveBases[1]].bSubtIndx;

	PPOITE pRetVal=(PPOITE)new char[sizeof(POITE)+iActNum*sizeof(RXINFOE)];
	pRetVal->blh[0].dLat=m_blhViewPoint.dLat; // RAD
	pRetVal->blh[0].dLon=m_blhViewPoint.dLon; // RAD
	pRetVal->blh[0].dHei=m_blhViewPoint.dHei; // RAD
	for (int iPos=0; iPos<pCtrl->p.dwPosCount; iPos++) {
		pRetVal->blh[iPos+1].dLat=pCtrl->p.positions[iPos].blh.dLat*DEG_TO_RAD;
		pRetVal->blh[iPos+1].dLon=pCtrl->p.positions[iPos].blh.dLon*DEG_TO_RAD;
		pRetVal->blh[iPos+1].dHei=pCtrl->p.positions[iPos].blh.dHei;
	}
	pRetVal-> uFlags = POIT_FLAGS_BLH_VALID | POIT_FLAGS_INF_VALID |
				POIT_FLAGS_RX_VALID | POIT_FLAGS_VIEWP_VALID;// | POIT_FLAGS_SECTOR_VALID;
	pRetVal-> iSector = 0;
	pRetVal-> uqTlock = 0;
	pRetVal-> iSmodeAdr = -1;
	memset(&pRetVal-> bSmodeCall[0], 0, sizeof(pRetVal -> bSmodeCall));
	pRetVal->Count=iActNum ; // two matches always
	PRXINFOE prx = &pRetVal-> rx[0];
	for (int iAct=0; iAct<iActNum; iAct++) {
		BLH blhMinu,blhSubt;
		XYZ xyzMinu,xyzSubt,xyzTg;
		int iBase = iActiveBases[iAct];
		int iMinuIdx=pCtrl->p.bases[iBase].bMinuIndx;
		int iSubtIdx=pCtrl->p.bases[iBase].bSubtIndx;
		memset(&prx -> bModeSData[0], 0, sizeof(prx -> bModeSData));
		prx -> usSifMode = 0;
		prx -> usFlags = 0;
		prx -> uT = 0;
		prx -> iIndex = 0;
		prx -> dF = 0.0e0;
		prx -> dAmp = 0.0e0;
		prx -> dTau = 0.0e0;
		prx -> sD = 70.0e0;
		prx -> uMinuIndx=iMinuIdx+1;
		prx -> uSubtIndx=iSubtIdx+1;
		blhMinu=pRetVal->blh[iMinuIdx+1];
		blhSubt=pRetVal->blh[iSubtIdx+1];
		blhMinu.dLat*=RAD_TO_DEG; blhMinu.dLon*=RAD_TO_DEG;
		blhSubt.dLat*=RAD_TO_DEG; blhSubt.dLon*=RAD_TO_DEG;
		m_pGeoUtils->BlhToXyz(&blhMinu,&xyzMinu);
		m_pGeoUtils->BlhToXyz(&blhSubt,&xyzSubt);
		BLH blhTg=m_blhTg;
		blhTg.dLat*=RAD_TO_DEG; blhTg.dLon*=RAD_TO_DEG;
		m_pGeoUtils->BlhToXyz(&blhTg,&xyzTg);
		double xx,yy,zz,rr,dRMinu,dRSubt;
		xx=xyzTg.dX-xyzMinu.dX; yy=xyzTg.dY-xyzMinu.dY; zz=xyzTg.dZ-xyzMinu.dZ;
		dRMinu=xx*xx+yy*yy+zz*zz; dRMinu=sqrt(dRMinu);
		xx=xyzTg.dX-xyzSubt.dX; yy=xyzTg.dY-xyzSubt.dY; zz=xyzTg.dZ-xyzSubt.dZ;
		dRSubt=xx*xx+yy*yy+zz*zz; dRSubt=sqrt(dRSubt);
		prx -> D = dRMinu - dRSubt; // path difference (meters). Cf.QPsPoi.cpp
		double dNoise=m_dGenSigmaM * NR::gasdev(m_idum);
		prx -> D+= dNoise;
		// qDebug() << " PathDiff=" << (dRMinu - dRSubt) << " dNoise=" << dNoise << " Hei=" << m_blhTg.dHei;
		prx++;
	}
	// qDebug() << " " << pRetVal->rx[0].uMinuIndx-1 << " " << pRetVal->rx[0].uSubtIndx-1
	//	     << " " << pRetVal->rx[1].uMinuIndx-1 << " " << pRetVal->rx[1].uSubtIndx-1;
	return(pRetVal);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGenerator::resetTime(qint64 iTime) {
	m_iTime=iTime;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGenerator::resetTarget(PBLH pblhTg) {
	m_blhTg = *pblhTg;
}

