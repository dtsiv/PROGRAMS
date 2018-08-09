#include "qvoiprocessor.h"
#include "tpoit.h"
#include "qgeoutils.h"
#include "qpoimodel.h"
#include "rmoexception.h"
// #include <cmath>
#include "nr.h"
using namespace std;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::addItem(QFile &qf, double dVal, bool bIndent, bool bNewline, int iPrecision /* =3 */) {
	if (bNewline) {
		qf.write("\n");
		return;
	}
	QString qs;
	if (bIndent) qs="\t";
	qs+=QString("%1").arg(dVal,10,'e',iPrecision,' ');
	qf.write(qs.toAscii().data());
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
void QVoiProcessor::refreshMatrixR(PPOITE pPoite) {
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");
	if (m_pR) { delete m_pR; m_pR=NULL; }
    m_pR = new Matrix(pPoite->Count,pPoite->Count);
    Matrix mKalR=(*m_pR);

	// measurement noise 
	double dSm2=m_dFltSigmaM*m_dFltSigmaM;
	mKalR=0;
	for (int i=0; i<pPoite->Count; i++) {
        mKalR[i][i]= dSm2;
	}

	(*m_pR) = mKalR;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshGeodeticParams() {
	// posterior state estimate \hat{x}_{k-1(+)}
	Matrix mXp=(*m_pXp);

	m_dLat      = mXp[0][0];
	m_dLon      = mXp[2][0];
	m_dTgW1     = mXp[3][0];
	m_dTgW2     = mXp[1][0];

	m_dGeoEps   = m_geoUtils.DEFMINOR/m_geoUtils.DEFMAJOR; 
	m_dGeoEps2  = 1.0e0-m_dGeoEps*m_dGeoEps; 
	m_dGeoEps   = sqrt(m_dGeoEps2);
	m_dGeoSinB  = sin(m_dLat);
	m_dGeoCosB  = cos(m_dLat); if (abs(m_dGeoCosB) < m_geoUtils.EPS) return;
	m_dGeoSin2B = 2*m_dGeoSinB*m_dGeoCosB;
    m_dGeoTgB   = m_dGeoSinB/m_dGeoCosB;
	m_dGeoSinL  = sin(m_dLon);
	m_dGeoCosL  = cos(m_dLon);
	m_dGeoXi    = 1.0e0-m_dGeoEps2*m_dGeoSinB*m_dGeoSinB; m_dGeoXi=1.0e0/sqrt(m_dGeoXi);
    m_dGeoXi3   = m_dGeoXi*m_dGeoXi*m_dGeoXi;
    m_dGeoXi5   = m_dGeoXi3*m_dGeoXi*m_dGeoXi;
	m_dGeoXiPrime = 0.5e0*m_dGeoEps2*m_dGeoXi3*m_dGeoSin2B;
    m_dGeoE     = 1.0e0-m_dGeoEps2; m_dGeoE=m_dGeoE*m_geoUtils.DEFMAJOR*m_dGeoXi3 + m_dHei;
	m_dGeoE2    = m_dGeoE*m_dGeoE;
	m_dGeoEPrime= 1.0e0-m_dGeoEps2; m_dGeoEPrime=1.5e0*m_dGeoEPrime*m_dGeoEps2*m_geoUtils.DEFMAJOR*m_dGeoXi5*m_dGeoSin2B;
    m_dGeoG     = m_geoUtils.DEFMAJOR*m_dGeoXi + m_dHei;
	m_dGeoG2    = m_dGeoG*m_dGeoG;
    m_dGeoGPrime= 0.5e0*m_dGeoEps2*m_geoUtils.DEFMAJOR*m_dGeoXi3*m_dGeoSin2B;

	m_dKalf1    = m_dTgW2/m_dGeoE;
	m_dKalf3    = m_dTgW1/m_dGeoG/m_dGeoCosB;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshMatrixF() {
	Matrix mKalF= (*m_pF);

	mKalF[0][0]= -m_dTgW2/m_dGeoE2*m_dGeoEPrime;
	mKalF[0][1]= 1.0e0/m_dGeoE;
	mKalF[0][2]= 0.0e0;
	mKalF[0][3]= 0.0e0;

	mKalF[1][0]= 0.0e0;
	mKalF[1][1]= 0.0e0;
	mKalF[1][2]= 0.0e0;
	mKalF[1][3]= 0.0e0;

	mKalF[2][0]= m_dKalf3*m_dGeoTgB-m_dTgW1/m_dGeoG2*m_geoUtils.DEFMAJOR*m_dGeoXi3*m_dGeoEps2*m_dGeoSinB;
	mKalF[2][1]= 0.0e0;
	mKalF[2][2]= 0.0e0;
	mKalF[2][3]= 1.0e0/m_dGeoG/m_dGeoCosB;

	mKalF[3][0]= 0.0e0;
	mKalF[3][1]= 0.0e0;
	mKalF[3][2]= 0.0e0;
	mKalF[3][3]= 0.0e0;

	(*m_pF) = mKalF;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshMatrixQ(double dTimeDiff) {
	Matrix mKalQ=*m_pQ;

	// system noise covariation
	double dSs2=m_dFltSigmaS2;
	double dDt2=dTimeDiff*dTimeDiff;
	double dDt3=dDt2*dTimeDiff;
	if (!m_bQinitialized) {
		// m_bQinitialized=true;
		mKalQ[0][0]= (1.0e0/3.0e0)*dSs2*dDt3/m_dGeoE2;
		mKalQ[0][1]= mKalQ[1][0]= (1.0e0/2.0e0)*dSs2*dDt2/m_dGeoE;
		mKalQ[0][2]= mKalQ[0][3]= 0.0e0;
		mKalQ[1][1]=               dSs2*dTimeDiff;
		mKalQ[1][2]= mKalQ[1][3]= 0.0e0;
		mKalQ[2][0]= mKalQ[2][1]= 0.0e0;
		mKalQ[2][2]= (1.0e0/3.0e0)*dSs2*dDt3/m_dGeoG2/m_dGeoCosB/m_dGeoCosB;
		mKalQ[2][3]= mKalQ[3][2]= (1.0e0/2.0e0)*dSs2*dDt2/m_dGeoG/m_dGeoCosB;
		mKalQ[3][0]= mKalQ[3][1]= 0.0e0;
		mKalQ[3][3]=               dSs2*dTimeDiff;

		*m_pQ=mKalQ;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::assign_mZ(PPOITE pPoite) {
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");
	if (m_pZ) { delete m_pZ; m_pZ=NULL; }
    m_pZ = new Matrix(pPoite->Count,1);

	Matrix mZ = (*m_pZ);
	mZ = 0;
	for (int i=0; i<pPoite->Count; i++) {
	    mZ[i][0]=pPoite->rx[i].D;
	}
	(*m_pZ) = mZ;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QVoiProcessor::spaceStrob(qint64 iTk, PPOITE pPoite) {
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");

    //------------ max spread (m) in topocentric plane -------------------------
	double dStrobSpreadMin=m_dStrobSpreadMin*1.0e3; // min strob (m) 
	double dStrobSpread=dStrobSpreadMin+m_dStrobSpreadSpeed*m_dDeltaT;

    //------------ normal direction --------------------------------------------
	double dXn = m_dGeoCosB*m_dGeoCosL;
	double dYn = m_dGeoCosB*m_dGeoSinL;
	double dZn = m_dGeoSinB;

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

	XYZ xyzPost,xyzPost1,xyzTg;
	BLH blhPost,blhPost1,blhTg;
	double xx,yy,zz;
	double dR1,dR;

	// central point
	blhTg.dLat = mXm[0][0]*RAD_TO_DEG; 
	blhTg.dLon = mXm[2][0]*RAD_TO_DEG; 
	blhTg.dHei = m_dHei;
	m_geoUtils.BlhToXyz(&blhTg,&xyzTg);

	for (int iMatch=0; iMatch<pPoite->Count; iMatch++) {

	    // pPoite->rx[].D = Minu - Subt
		int iMinu=pPoite->rx[iMatch].uMinuIndx;
		int iSubt=pPoite->rx[iMatch].uSubtIndx;
		blhPost  = pPoite->blh[iMinu];
		blhPost.dLat*=RAD_TO_DEG; blhPost.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost,&xyzPost);
		blhPost1 = pPoite->blh[iSubt];
		blhPost1.dLat*=RAD_TO_DEG; blhPost1.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost1,&xyzPost1);

		// gradient of pathdiff measurement over geocentric coordinates
        double dGradX,dGradY,dGradZ;
		xx=xyzTg.dX-xyzPost.dX;
		yy=xyzTg.dY-xyzPost.dY;
		zz=xyzTg.dZ-xyzPost.dZ;
		dR=sqrt(xx*xx+yy*yy+zz*zz);
		dGradX=xx/dR;
		dGradY=yy/dR;
		dGradZ=zz/dR;

		xx=xyzTg.dX-xyzPost1.dX;
		yy=xyzTg.dY-xyzPost1.dY;
		zz=xyzTg.dZ-xyzPost1.dZ;
		dR1=sqrt(xx*xx+yy*yy+zz*zz);
		dGradX=dGradX-xx/dR1;
		dGradY=dGradY-yy/dR1;
		dGradZ=dGradZ-zz/dR1;

		// magnitude of topocentric lateral (tangent) projection of measurement gradient
        double dTanGrad;
		xx=dGradX*(1.0e0-dXn); yy=dGradY*(1.0e0-dYn); zz=dGradZ*(1.0e0-dZn);
        dTanGrad=sqrt(xx*xx+yy*yy+zz*zz);

		// spread of path difference measurement
        double dPathDiffSpread=dTanGrad*dStrobSpread;
		double dPathDiff=dR-dR1;
		double dPathDiffOffset=abs(dPathDiff-pPoite->rx[iMatch].D);

		if (dPathDiffOffset>dPathDiffSpread) {
			qDebug() << "dDeltaT,iMatch,dTanGrad,dPathDiffOffset,dPathDiffSpread=" 
					 << " " << m_dDeltaT
					 << " " << iMatch
					 << " " << dTanGrad
					 << " " << dPathDiffOffset
					 << " " << dPathDiffSpread;
			return false;
		}
	}
	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::matrixDebugOutput(bool bAppend, int iTk /* =0 */ ) {
	QFile qfMaytixP("matrixp.txt");
	QFile qfMatrixH("matrixh.txt");
	QFile qfVectorZ("vectorz.txt");
	QFile qfKalK("kalmank.txt");
	QFile qfTraj("traj.txt");        
	QFile qfDenDet("dendet.txt");
	QFile qfChi2("chi2.txt");
	QFile qfPeigen("peigen.txt");
	QFile qfTildeZ("tildez.txt");

	if (!bAppend) {
		qfMaytixP.resize(0);
		qfMatrixH.resize(0);
		qfVectorZ.resize(0);
		qfKalK.resize(0);
		qfTraj.resize(0);
		qfDenDet.resize(0);
		qfChi2.resize(0);
		qfPeigen.resize(0);
		qfTildeZ.resize(0);
		return;
	}
    //---------------------------------- debug output ------------------------------------------------
	qfMaytixP.open(QIODevice::ReadWrite);
	qfMaytixP.seek(qfMaytixP.size());
	addItem(qfMaytixP,iTk*1.0e-3,false,false,20);
	Matrix mPp=(*m_pPp);
	for (int i=0; i<mPp.colsize(); i++) {
		for (int j=0; j<mPp.rowsize(); j++) {
			addItem(qfMaytixP,mPp[i][j],true,false);
		}
	}
	addItem(qfMaytixP,0,true,true);
	qfMaytixP.close();

	int nSamp=m_qlZ[0].count();
	if (nSamp && m_qlZ[1].count()==nSamp && m_qlZ[2].count()==nSamp) {
		qfTildeZ.open(QIODevice::ReadWrite);
		qfTildeZ.seek(qfTildeZ.size());
		addItem(qfTildeZ,iTk*1.0e-3,false,false,20);
	    double dProj;
  		dProj=-0.120682*m_qlZ[0].last()   -0.925959*m_qlZ[1].last()   +0.357821 *m_qlZ[2].last();
			addItem(qfTildeZ,dProj,true,false);
  		dProj=-0.710985*m_qlZ[0].last()   -0.170929*m_qlZ[1].last()   -0.682117 *m_qlZ[2].last();
			addItem(qfTildeZ,dProj,true,false);
  		dProj=-0.692774*m_qlZ[0].last()   +0.336725*m_qlZ[1].last()   +0.637715 *m_qlZ[2].last();
			addItem(qfTildeZ,dProj,true,false);
			addItem(qfTildeZ,0,true,true);
		qfTildeZ.close();
	}

	qfMatrixH.open(QIODevice::ReadWrite);
	qfMatrixH.seek(qfMatrixH.size());
	addItem(qfMatrixH,iTk*1.0e-3,false,false,20);
	Matrix mKalH=(*m_pH);
	for (int i=0; i<mKalH.colsize(); i++) {
		for (int j=0; j<mKalH.rowsize(); j++) {
			if (!m_pH) return;
			addItem(qfMatrixH,mKalH[i][j],true,false);
		}
	}
	addItem(qfMatrixH,0,true,true);
	qfMatrixH.close();

	qfVectorZ.open(QIODevice::ReadWrite);
	qfVectorZ.seek(qfVectorZ.size());
	addItem(qfVectorZ,iTk*1.0e-3,false,false,20);
	Matrix mZ=(*m_pZ);
	for (int i=0; i<mZ.colsize(); i++) {
		for (int j=0; j<mZ.rowsize(); j++) {
			if (!m_pZ) return;
			addItem(qfVectorZ,mZ[i][j],true,false);
		}
	}
	addItem(qfVectorZ,0,true,true);
	qfVectorZ.close();

	qfKalK.open(QIODevice::ReadWrite);
	qfKalK.seek(qfKalK.size());
	addItem(qfKalK,iTk*1.0e-3,false,false,20);
	Matrix mKalK=(*m_pK);
	for (int i=0; i<mKalK.colsize(); i++) {
		for (int j=0; j<mKalK.rowsize(); j++) {
			if (!m_pK) return;
			addItem(qfKalK,mKalK[i][j],true,false);
		}
	}
	addItem(qfKalK,0,true,true);
	qfKalK.close();

	qfDenDet.open(QIODevice::ReadWrite);
	qfDenDet.seek(qfDenDet.size());
	addItem(qfDenDet,iTk*1.0e-3,false,false,20);
	addItem(qfDenDet,m_dDet,true,false);
	addItem(qfDenDet,0,true,true);
	qfDenDet.close();

	{
		BLH blhTg;
		XYZ xyzTg;
		if (!m_pXp) return;
	    // posterior state estimate \hat{x}_{k(+)}
		Matrix mXp=(*m_pXp); 

		blhTg.dLat = mXp[0][0]; blhTg.dLon = mXp[2][0]; blhTg.dHei = m_dHei;
		m_geoUtils.toTopocentric(&m_blhViewPoint,&blhTg,&xyzTg);
		qfTraj.open(QIODevice::ReadWrite);
		qfTraj.seek(qfTraj.size());
		addItem(qfTraj,iTk*1.0e-3,false,false,20);
		addItem(qfTraj,xyzTg.dX,true,false);
		addItem(qfTraj,xyzTg.dY,true,false);
		addItem(qfTraj,0,false,true);
		qfTraj.close();
	}

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshMatrixHnum(PPOITE pPoite) {
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");
	if (m_pHnum) { delete m_pHnum; m_pHnum=NULL; }
	m_pHnum = new Matrix(pPoite->Count,4);
	Matrix mHnum = (*m_pHnum);
	mHnum = 0;

	// calculation of matrix H
	XYZ xyzPost,xyzPost1,xyzTg;
	BLH blhPost,blhPost1,blhTg;
	double dAngleIncr = 1.0e-4;
	double xx,yy,zz;
	double dR1,dR;
	// posterior state estimate \hat{x}_{k-1(+)}
	Matrix mXm=(*m_pXm);

	for (int iMatch=0; iMatch<pPoite->Count; iMatch++) {
	    // pPoite->rx[].D = Minu - Subt
		int iMinu=pPoite->rx[iMatch].uMinuIndx;
		int iSubt=pPoite->rx[iMatch].uSubtIndx;
		blhPost  = pPoite->blh[iMinu];
		blhPost.dLat*=RAD_TO_DEG; blhPost.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost,&xyzPost);
		blhPost1 = pPoite->blh[iSubt];
		blhPost1.dLat*=RAD_TO_DEG; blhPost1.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost1,&xyzPost1);

		// central point
		blhTg.dLat = mXm[0][0]*RAD_TO_DEG; 
		blhTg.dLon = mXm[2][0]*RAD_TO_DEG; 
		blhTg.dHei = m_dHei;
		m_geoUtils.BlhToXyz(&blhTg,&xyzTg);

		xx=xyzTg.dX-xyzPost.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost.dZ; zz=zz*zz;
		dR=sqrt(xx+yy+zz);

		xx=xyzTg.dX-xyzPost1.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost1.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost1.dZ; zz=zz*zz;
		dR1=sqrt(xx+yy+zz);

		mHnum[iMatch][0]=dR-dR1;
		mHnum[iMatch][2]=dR-dR1;

	    // increment X_0
		blhTg.dLat = (mXm[0][0] + dAngleIncr)*RAD_TO_DEG; 
		blhTg.dLon = mXm[2][0]*RAD_TO_DEG; 
		blhTg.dHei = m_dHei;
		m_geoUtils.BlhToXyz(&blhTg,&xyzTg);

		xx=xyzTg.dX-xyzPost.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost.dZ; zz=zz*zz;
		dR=sqrt(xx+yy+zz);

		xx=xyzTg.dX-xyzPost1.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost1.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost1.dZ; zz=zz*zz;
		dR1=sqrt(xx+yy+zz);

	    mHnum[iMatch][0]=(dR-dR1-mHnum[iMatch][0]) / dAngleIncr;

    	// increment X_2
		blhTg.dLat = mXm[0][0]*RAD_TO_DEG; 
		blhTg.dLon = (mXm[2][0] + dAngleIncr)*RAD_TO_DEG; 
		blhTg.dHei = m_dHei;
		m_geoUtils.BlhToXyz(&blhTg,&xyzTg);

		xx=xyzTg.dX-xyzPost.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost.dZ; zz=zz*zz;
		dR=sqrt(xx+yy+zz);

		xx=xyzTg.dX-xyzPost1.dX; xx=xx*xx;
		yy=xyzTg.dY-xyzPost1.dY; yy=yy*yy;
		zz=xyzTg.dZ-xyzPost1.dZ; zz=zz*zz;
		dR1=sqrt(xx+yy+zz);

	    mHnum[iMatch][2]=(dR-dR1-mHnum[iMatch][2]) / dAngleIncr;
	}

	(*m_pHnum) = mHnum;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshMatrixH(PPOITE pPoite) {
	// security check
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");

	// memory cleanup
	if (m_pH) { delete m_pH; m_pH=NULL; }
	m_pH = new Matrix(pPoite->Count,4);
	if (m_pZm) { delete m_pZm; m_pZm=NULL; }
	m_pZm = new Matrix(pPoite->Count,1);

	// measurement sensitivity matrix H_k
	Matrix mKalH = (*m_pH);
	mKalH = 0;

	//-------------- calculation of matrix H ---------------
	// prior state estimate \hat{x}_{k(-)}
	Matrix mXm=(*m_pXm);
	// prior measurement estimate \hat{z}_{k(-)}
	Matrix mZm=(*m_pZm);
    mZm = 0;
    mZm = 0;

	// to Cartesian geocentric coordinates \hat{x}_{k(-)}
	XYZ xyzTgPosterior;
	BLH blhTg;
	blhTg.dLat = mXm[0][0]*RAD_TO_DEG; 
	blhTg.dLon = mXm[2][0]*RAD_TO_DEG; 
	blhTg.dHei = m_dHei;
	m_geoUtils.BlhToXyz(&blhTg,&xyzTgPosterior);

	XYZ xyzPost,xyzPost1;
	BLH blhPost,blhPost1;
	for (int iMatch=0; iMatch<pPoite->Count; iMatch++) {
	    // pPoite->rx[].D = Minu - Subt
		int iMinu=pPoite->rx[iMatch].uMinuIndx;
		int iSubt=pPoite->rx[iMatch].uSubtIndx;
		blhPost  = pPoite->blh[iMinu];
		blhPost.dLat*=RAD_TO_DEG; blhPost.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost,&xyzPost);
		blhPost1 = pPoite->blh[iSubt];
		blhPost1.dLat*=RAD_TO_DEG; blhPost1.dLon*=RAD_TO_DEG;
		m_geoUtils.BlhToXyz(&blhPost1,&xyzPost1);

		double xx,yy,zz;
		xx=xyzTgPosterior.dX-xyzPost.dX; xx=xx*xx;
		yy=xyzTgPosterior.dY-xyzPost.dY; yy=yy*yy;
		zz=xyzTgPosterior.dZ-xyzPost.dZ; zz=zz*zz;
		double dR=sqrt(xx+yy+zz);
		xx=xyzTgPosterior.dX-xyzPost1.dX; xx=xx*xx;
		yy=xyzTgPosterior.dY-xyzPost1.dY; yy=yy*yy;
		zz=xyzTgPosterior.dZ-xyzPost1.dZ; zz=zz*zz;
		double dR1=sqrt(xx+yy+zz);

		// measurement a posteriori estimation: \mathcal{D}(\hat{x}_{(k-1)(+)})
		// which is same as \hat{z}_{(k-1)(+)}
		mZm[iMatch][0]= dR-dR1;

		// further intermediate calculus for matrix H
		xx=xyzTgPosterior.dX-xyzPost.dX; 
		double ddRdx=xx/dR;
		yy=xyzTgPosterior.dY-xyzPost.dY; 
		double ddRdy=yy/dR;
		zz=xyzTgPosterior.dZ-xyzPost.dZ; 
		double ddRdz=zz/dR;
		xx=xyzTgPosterior.dX-xyzPost1.dX; 
		double ddR1dx=xx/dR1;
		yy=xyzTgPosterior.dY-xyzPost1.dY; 
		double ddR1dy=yy/dR1;
		zz=xyzTgPosterior.dZ-xyzPost1.dZ; 
		double ddR1dz=zz/dR1;
		double dAXiPlusH = m_geoUtils.DEFMAJOR*m_dGeoXi + m_dHei;
		double dA1mEpsXiPlusH = 1.0e0-m_dGeoEps2; dA1mEpsXiPlusH=m_geoUtils.DEFMAJOR*dA1mEpsXiPlusH*m_dGeoXi + m_dHei;

		double ddXdB=m_geoUtils.DEFMAJOR*m_dGeoXiPrime*m_dGeoCosB*m_dGeoCosL-dAXiPlusH*m_dGeoSinB*m_dGeoCosL;
		double ddYdB=m_geoUtils.DEFMAJOR*m_dGeoXiPrime*m_dGeoCosB*m_dGeoSinL-dAXiPlusH*m_dGeoSinB*m_dGeoSinL;
		double ddZdB=m_geoUtils.DEFMAJOR*m_dGeoEps2*m_dGeoXiPrime*m_dGeoSinB+dA1mEpsXiPlusH*m_dGeoCosB;

		double ddXdL=-dAXiPlusH*m_dGeoCosB*m_dGeoSinL;
		double ddYdL= dAXiPlusH*m_dGeoCosB*m_dGeoCosL;
		double ddZdL= 0.0e0;

		double ddXdH=m_dGeoCosB*m_dGeoCosL;
		double ddYdH=m_dGeoCosB*m_dGeoSinL;
		double ddZdH=m_dGeoSinB;

		mKalH[iMatch][0]=(ddRdx-ddR1dx)*ddXdB+(ddRdy-ddR1dy)*ddYdB+(ddRdz-ddR1dz)*ddZdB;
		mKalH[iMatch][1]=0.0e0;
		mKalH[iMatch][2]=(ddRdx-ddR1dx)*ddXdL+(ddRdy-ddR1dy)*ddYdL+(ddRdz-ddR1dz)*ddZdL;
		mKalH[iMatch][3]=0.0e0;
	}
	// posterior measurement estimate \hat{z}_{k-1(+)}
	(*m_pZm)=mZm;
	// measurement sensitivity matrix H_k
	(*m_pH) = mKalH;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::refreshMatrixK(PPOITE pPoite) {
	if (pPoite->Count<0 || pPoite->Count>3) throw RmoException("pPoite->Count out of range!");
	if (m_pK) { delete m_pK; m_pK=NULL; }
	m_pK = new Matrix(4,pPoite->Count);
	Matrix mKalK = (*m_pK);
	mKalK = 0;

	Matrix mKalH = (*m_pH);
	Matrix mKalPm= (*m_pPm);
	Matrix mKalR = (*m_pR);

	Matrix mDenominator(pPoite->Count,pPoite->Count);
	mDenominator= mKalH * mKalPm * (~mKalH) + mKalR;
	// det
	m_dDet=mDenominator.det();
	if (abs(m_dDet)<QVOIPROCESSOR_EPS) throw RmoException(QString("QVoiProcessor: det=%1").arg(m_dDet));

	mKalK=mKalPm*(~mKalH)*(!mDenominator);
	(*m_pK) = mKalK;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::matrixRelaxation(double dDuration) {
    refreshMatrixQ(m_dDeltaT);
	Matrix mKalPp= (*m_pPp);
	Matrix mKalPm= mKalPp;
	Matrix mKalF = (*m_pF);
	Matrix mKalH = (*m_pH);
	Matrix mKalK = (*m_pK);
	Matrix mKalR = (*m_pR);
	Matrix mKalQ = (*m_pQ);

	Matrix mDenominator(mKalK.rowsize(),mKalK.rowsize());
	double dCurT=0.0e0; // blind variable
	while (dCurT<dDuration) {
		//------------ increment current time ------------
		dCurT += m_dDeltaT;
		matrixEvolution(m_dDeltaT);
		//------------ prior covariations ----------------
	    mKalPm =(*m_pPm);
		mKalPm = mKalPm+mKalQ;
		//--------------- Kalman gain --------------------
		mDenominator= mKalH * mKalPm * (~mKalH) + mKalR;
		m_dDet=mDenominator.det();
		if (abs(m_dDet)<QVOIPROCESSOR_EPS) throw RmoException(QString("QVoiProcessor: det=%1").arg(m_dDet));
		mKalK=mKalPm*(~mKalH)*(!mDenominator);
		//--------------- system noise posterior covariation P_{k(+)} ------------------
		double dIdentity[]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
		Matrix mBracket(4,4,dIdentity);
		mBracket= mBracket-mKalK*mKalH;
		mKalPp = mBracket * mKalPm * (~mBracket)
			+ mKalK * mKalR * (~mKalK);
	    (*m_pPp)= mKalPp;
	}
	//------------ save current covariation --------------
	(*m_pPm)= mKalPm;
	(*m_pPp)= mKalPp;
	(*m_pK) = mKalK;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::matrixEvolution(double dDuration) {
	Matrix mKalF = (*m_pF);
	Matrix mKalPp= (*m_pPp);
	Matrix mKalPm= mKalPp;
    Matrix mIncrement(4,4);
	double dCurT=0.0e0; // blind variable
	while (dCurT+m_dIntegrStep<dDuration) {
		//------------ increment current time ------------
		dCurT += m_dIntegrStep;
		//------------ prior covariations ----------------
	    mIncrement=mKalF*mKalPm;
	    mIncrement=mIncrement+mKalPm*(~mKalF);
		mKalPm = mKalPm+mIncrement*m_dIntegrStep;
	}
    //------------------- last step ----------------------
    mIncrement=mKalF*mKalPm;
    mIncrement=mIncrement+mKalPm*(~mKalF);
	mKalPm = mKalPm+mIncrement*(dDuration-dCurT);
	//------------ save current covariation --------------
	(*m_pPm)=mKalPm;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::covEigenvectors(int iTk /* =0 */ ) {
    const int NP=4;
    const DP TINY=1.0e-6;
    int i,j,k;
    Vec_DP d(NP),e(NP),f(NP);
    Mat_DP a(NP,NP),c(NP,NP);
    // posterior covariation matrix
	// double dArr[]={0,3,0,0,3,0,0,0,0,0,-4,0,0,0,0,1};
    // Matrix mKalPp(NP,NP,dArr);
	Matrix mKalPp=(*m_pPp);
	if (mKalPp.colsize()!=NP || mKalPp.rowsize()!=NP) {
		throw RmoException("mKalPp.colsize()!=NP");
	}
    // initialize local variables
	for (i=0; i<NP; i++) {
	    for (j=0; j<NP; j++) {
            a[i][j]=c[i][j]=mKalPp[i][j];
    	}
	}
    NR::tred2(a,d,e);
    NR::tqli(d,e,a);
    // tsMat << fixed << setprecision(6);
	QString qsEiegenvector;
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
	QFile qfPeigen("peigen.txt");
	qfPeigen.open(QIODevice::ReadWrite);
	qfPeigen.seek(qfPeigen.size());
	QString qsOut("%1\t%2\t%3\t%4\t%5\t%6\t%7");
	qfPeigen.write(
		qsOut.arg(iTk*1.0e-3,0,'g',5)
		     .arg(d[0],0,'g',8)
		     .arg(d[1],0,'g',8)
		     .arg(a[2][0],0,'g',8)
			 .arg(a[0][0],0,'g',8)
			 .arg(a[2][1],0,'g',8)
			 .arg(a[0][1],0,'g',8)
		 .toAscii().data()
    );
	qfPeigen.write("\n");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QVoiProcessor::exactStatistics() {
    int i,j,k;
	QString qs;
	qs+=QString::number(m_dFltSigmaM)+"\t";
	{
		const int NP=4;
		const DP TINY=1.0e-6;
		int i,j,k;
		Vec_DP d(NP),e(NP),f(NP);
		Mat_DP a(NP,NP),c(NP,NP);
		// posterior covariation matrix
		// double dArr[]={0,3,0,0,3,0,0,0,0,0,-4,0,0,0,0,1};
		// Matrix mKalPp(NP,NP,dArr);
		Matrix mKalPp=(*m_pPp);
		if (mKalPp.colsize()!=NP || mKalPp.rowsize()!=NP) {
			throw RmoException("mKalPp.colsize()!=NP");
		}
		// initialize local variables
		for (i=0; i<NP; i++) {
			for (j=0; j<NP; j++) {
				a[i][j]=c[i][j]=mKalPp[i][j];
    		}
		}
		NR::tred2(a,d,e);
		NR::tqli(d,e,a);
		// tsMat << fixed << setprecision(6);
		QString qsEiegenvector;
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
		qs+=QString::number(d[0])+"\t";
		qs+=QString::number(d[1])+"\t";
	}
    //-------------------- tilde z covariations --------------------
	int nSamp=m_qlZ[0].count();
	if (nSamp>1 && nSamp==m_qlZ[1].count() && nSamp==m_qlZ[2].count()) {
		const int NP=3;
		const DP TINY=1.0e-6;
		Vec_DP d(NP),e(NP),f(NP);
		Mat_DP a(NP,NP),c(NP,NP);
		double dAvrTidleZ[NP]={0.0,0.0,0.0};
		Matrix mCovTildeZ(NP,NP);
		for (i=0; i<NP; i++) {
		    for (k=0; k<nSamp; k++) {
			    dAvrTidleZ[i]+=m_qlZ[i].at(k);
			}
			dAvrTidleZ[i]/=nSamp;
		}
		for (i=0; i<NP; i++) {
		    for (j=0; j<NP; j++) {
				mCovTildeZ[i][j]=0.0e0;
				for (k=0; k<nSamp; k++) {
					double dDiffI=m_qlZ[i].at(k)-dAvrTidleZ[i];
					double dDiffJ=m_qlZ[j].at(k)-dAvrTidleZ[j];
					mCovTildeZ[i][j]+=dDiffI*dDiffJ;
				}
			    a[i][j]=mCovTildeZ[i][j]/(nSamp-1);
			}
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
		qDebug() << "Sample(" << nSamp << ") TildeZ covariation eval " << d[0] << " " << d[1] << " " << d[2];
		for (i=0; i<NP; i++) {
			double dProj=0.0e0;
		    for (j=0; j<NP; j++) {
			    dProj+=dAvrTidleZ[i]*a[j][i];
			}
            qDebug() << "SampleAvrProj_" << i << " = " << dProj;
		}
        qDebug() << "evec[0] " << a[0][0] << " " << a[1][0] << " " << a[2][0];
        qDebug() << "evec[1] " << a[0][1] << " " << a[1][1] << " " << a[2][1];
        qDebug() << "evec[2] " << a[0][2] << " " << a[1][2] << " " << a[2][2];
	}

	//------------------- cov eigenval & eigenvec ------------------
	double dAvrErrB,dAvrErrL;
    dAvrErrB=0.0e0;
	dAvrErrL=0.0e0;
	int iMax=m_qlErrB.count();
	if (iMax<2 || m_qlErrB.count() != m_qlErrL.count()) throw RmoException("Error list len mismatch");
	for (i=0; i<iMax; i++) {
		dAvrErrB+=m_qlErrB.at(i);
		dAvrErrL+=m_qlErrL.at(i);
	}
	dAvrErrB/=iMax;
	dAvrErrL/=iMax;
    const int NP=2;
    const DP TINY=1.0e-6;
    Vec_DP d(NP),e(NP),f(NP);
    Mat_DP a(NP,NP),c(NP,NP);
	a[0][0]=a[1][1]=a[0][1]=a[1][0]=0.0e0;
	for(i=0;i<iMax;i++) {
		double a00=(m_qlErrB.at(i)-dAvrErrB);
		double a11=(m_qlErrL.at(i)-dAvrErrL);
		double a10=a00*a11;
		a[0][0]+=a00*a00;
		a[1][1]+=a11*a11;
		a[0][1]+=a10;
		a[1][0]+=a10;
	}
	for(i=0;i<NP;i++) for(j=0;j<NP;j++) {
	    a[i][j]/=(iMax-1);
	}
    qDebug() << "a[0] " << a[0][0] << " " << a[0][1];
    qDebug() << "a[1] " << a[1][0] << " " << a[1][1];
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
	qs+=QString::number(d[0])+"\t";
	qs+=QString::number(d[1])+"\n";
    qDebug() << "Sample(" << iMax << ") covariation eval " << d[0] << " " << d[1];
	double dAvr0=dAvrErrB*a[0][0]+dAvrErrL*a[1][0];
	double dAvr1=dAvrErrB*a[0][1]+dAvrErrL*a[1][1];
    qDebug() << "Avr (%sigma) " << dAvr0/sqrt(d[0])*sqrt(1.0*iMax) << " " << dAvr1/sqrt(d[1])*sqrt(1.0*iMax);
    qDebug() << "evec[0] " << a[1][0] << " " << a[0][0];
    qDebug() << "evec[1] " << a[1][1] << " " << a[0][1];
	QList<double> qlEst;
	for(i=0;i<iMax;i++) {
        qlEst << (a[0][1]*m_qlErrB.at(i)+a[1][1]*m_qlErrL.at(i));
	}
	for(i=0;i<iMax-1;i++) {
	    for(j=i+1;j<iMax;j++) {
			if (qlEst.at(j)<qlEst.at(i)) qlEst.swap(i,j);
		}
	}
	QFile qfDistr("estdistr.txt");
	qfDistr.resize(0);
	qfDistr.open(QIODevice::ReadWrite);
	for(i=0;i<iMax-1;i++) {
		qfDistr.write(QString("%1\t%2\n").arg(qlEst.at(i)).arg(1.0*i/iMax).toAscii());
	}
    qfDistr.close();
	QFile qfEigen("eigen.txt");
	qfEigen.open(QIODevice::ReadWrite);
	qfEigen.seek(qfEigen.size());
	qfEigen.write(qs.toAscii().data());
	qfEigen.close();
}

