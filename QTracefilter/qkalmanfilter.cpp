#include "qkalmanfilter.h"
#include "rmoexception.h"

// #define DEBUG_MATRIXK
// #define DEBUG_MATRXIP
#ifdef DEBUG_MATRXIP
bool bInRelax=false;
#endif

#define QKALMANFILTER_DUMMY_DEFAULT -9999999.9e99

double QKalmanFilter::m_dFltSigmaS2=QKALMANFILTER_DUMMY_DEFAULT;     // Kalman filter: system speed sigma2 m2/s3
double QKalmanFilter::m_dFltSigmaM=QKALMANFILTER_DUMMY_DEFAULT;      // Kalman filter: measurement sigma m
double QKalmanFilter::m_dFltHeight=QKALMANFILTER_DUMMY_DEFAULT;      // Kalman filter: height km
double QKalmanFilter::m_dIntegrStep=1.0e-3;                           // Time integration step s
double QKalmanFilter::m_dStrobSpreadSpeed=QKALMANFILTER_DUMMY_DEFAULT; // spread speed m/s
double QKalmanFilter::m_dStrobSpreadMin=QKALMANFILTER_DUMMY_DEFAULT;   // min spread km
int QKalmanFilter::m_iChi2Prob=0;                                   // chi2 statistics probability threshold
bool QKalmanFilter::m_bUseMatRelax=true;                              // use cov relaxation
double QKalmanFilter::m_dMatRelaxTime=QKALMANFILTER_DUMMY_DEFAULT;    // cov relax time
bool QKalmanFilter::m_bStickTrajToModeS=true;                         // stick trajectory to ModeS code
double QKalmanFilter::m_dClusterCutoff=QKALMANFILTER_DUMMY_DEFAULT;   // cluster cutoff radius (km)
int QKalmanFilter::m_iClusterMinSize=0;                            // minimum cluster size to start traj
double QKalmanFilter::m_dTrajMaxVelocity=QKALMANFILTER_DUMMY_DEFAULT; // max velocity (m/s) to kill traj
double QKalmanFilter::m_dTrajTimeout=QKALMANFILTER_DUMMY_DEFAULT;     // timeout (min) to kill traj
bool QKalmanFilter::m_bEstIniVelocity=true;                           // estimate V_ini from primary cluster

const int QKalmanFilter::CHI2_NUM_QUANTILES=QKALMANFILTER_CHI2_NUM_QUANTILES;
double QKalmanFilter::m_dChi2QuantProb[QKALMANFILTER_CHI2_NUM_QUANTILES]={
    0.2, 0.1, 0.05, 0.02, 0.01, 0.001
};
double QKalmanFilter::m_dChi2Quant[4][QKALMANFILTER_CHI2_NUM_QUANTILES] ={
// $\alpha=0.2$ & $\alpha=0.1$ & $\alpha=0.05$ & $\alpha=0.02$ & $\alpha=0.01$ & $\alpha=0.001$
    {0,             0,             0,              0,              0,              0 },
    {1.642,         2.706,         3.841,          5.412,          6.635,          10.827},     // m=1
    {3.219,         4.605,         5.991,          7.824,          9.210,          13.815},     // m=2
    {4.642,         6.251,         7.815,          9.837,          11.341,         16.268}      // m=3
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  blhTg: dLat, dLon (radians), dHei (meters); dVx, dVy (m/s)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QKalmanFilter::QKalmanFilter(qint64 iTk, const QGeoUtils &geoUtils, BLH blhTg,
                             double dVx /* =0.0e0 */, double dVy  /* =0.0e0 */) :
       m_geoUtils(geoUtils)
     , m_pPm(NULL)
     , m_pPp(NULL)
     , m_pQ(NULL)
     , m_pR(NULL)
     , m_pF(NULL)
     , m_pK(NULL)
     , m_pH(NULL)
     , m_pXp(NULL)
     , m_pXm(NULL)
     , m_pZ(NULL)
     , m_pZm(NULL)
     , m_bQinitialized(false)
     , m_bFinitialized(false)
     , m_bHinitialized(false)
     , m_iTlast (iTk)
     , m_iTStart (iTk)
     , m_iTimeout (m_dTrajTimeout*60000)
     , m_bMatRelaxed(false)
     , m_pMainCtrl(NULL)
     , m_qsSmodeAdr(QString())
     , m_qsSmodeCall(QString())
     , m_iSmodeAdr(-1)
     , m_qsModeSData(QString())
     {
    // check if m_geoUtils is actually initialized
    if (!m_geoUtils.m_pMainCtrl) throw RmoException("QKalmanFilter::QKalmanFilter QGeoUtils::m_pMainCtrl is NULL");
    m_pMainCtrl=m_geoUtils.m_pMainCtrl;
    MAINCTRL_P mp=m_pMainCtrl->p;
    if (mp.dwPosCount<5 || mp.dwPosCount>100) throw RmoException(QString("QKalmanFilter mp.dwPosCount = %1").arg(mp.dwPosCount));

    m_pMainCtrl=m_geoUtils.m_pMainCtrl;
    m_geoUtils.getViewPoint(&m_blhViewPoint,m_pMainCtrl);

	m_pPm = new Matrix(4,4);
	m_pPp = new Matrix(4,4);
	m_pQ = new Matrix(4,4);
	m_pF = new Matrix(4,4);
	// posterior state estimate \hat{x}_{k-1(+)}
	m_pXp = new Matrix(4,1);
	// prior state estimate \hat{x}_{k(-)}
	m_pXm = new Matrix(4,1);

	double dMatrixPeq[]={
        1.000e+00,	 0.000e+00,	 0.000e+00,	 0.000e+00,	 
        0.000e+00,	 1.000e+00,	 0.000e+00,	 0.000e+00,	 
        0.000e+00,	 0.000e+00,	 1.000e+00,	 0.000e+00,	 
        0.000e+00,	 0.000e+00,	 0.000e+00,	 1.000e+00
	};
	Matrix mMatrixPeq(4,4,dMatrixPeq);
	(*m_pPp)=mMatrixPeq;

    // initialize target position and velocity
    if (!m_pXp) throw RmoException("QKalmanFilter: m_pXp is NULL");
    Matrix mXp = (*m_pXp);
    mXp[0][0] = blhTg.dLat; // (radians)
    mXp[2][0] = blhTg.dLon; // (radians)
    mXp[1][0] = dVx; // (m/s)
    mXp[3][0] = dVy; // (m/s)
    (*m_pXp)= mXp;

    // initialize constant target height m_dHei (m) -- from static member m_dFltHeight (km)
    m_dHei = m_dFltHeight*1.0e3;

    // initialize auxiliary geodetic variables
    refreshGeodeticParams();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QKalmanFilter::~QKalmanFilter(void) {
	if (m_pPm) delete m_pPm;
	if (m_pPp) delete m_pPp;
	if (m_pQ) delete m_pQ;
	if (m_pR) delete m_pR;
	if (m_pF) delete m_pF;
	if (m_pK) delete m_pK;
	if (m_pH) delete m_pH;
	if (m_pXp) delete m_pXp;
	if (m_pXm) delete m_pXm;
	if (m_pZ) delete m_pZ;
	if (m_pZm) delete m_pZm;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QKalmanFilter::refreshGeodeticParams() {
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
void QKalmanFilter::refreshMatrixF() {
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
void QKalmanFilter::refreshMatrixR(PPOITE pPoite) {
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
void QKalmanFilter::refreshMatrixH(PPOITE pPoite) {
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
void QKalmanFilter::refreshMatrixK(PPOITE pPoite) {
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
    if (abs(m_dDet)<QKALMANFILTER_EPS) throw RmoException(QString("QVoiProcessor: det=%1").arg(m_dDet));

    mKalK=mKalPm*(~mKalH)*(!mDenominator);
    (*m_pK) = mKalK;
#ifdef DEBUG_MATRIXK
    qDebug() << "pPoite->Count=" << pPoite->Count << " m_dDet=" << m_dDet;
    qDebug() << "mKalPm";
    for (int i=0; i<mKalPm.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalPm.colno(); j++) {
            qs+=QString::number(mKalPm[i][j])+"  ,  ";
        }
        qDebug() << qs;
    }

    qDebug() << "mKalH";
    for (int i=0; i<mKalH.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalH.colno(); j++) {
            qs+=QString::number(mKalH[i][j])+"  ,  ";
        }
        qDebug() << qs;
    }
    qDebug() << "mDenominator";
    for (int i=0; i<mDenominator.rowno(); i++) {
        QString qs;
        for (int j=0; j<mDenominator.colno(); j++) {
            qs+=QString::number(mDenominator[i][j])+"  ,  ";
        }
        qDebug() << qs;
    }
    qDebug() << "mKalK";
    for (int i=0; i<mKalK.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalK.colno(); j++) {
            qs+=QString::number(mKalK[i][j])+"  ,  ";
        }
        qDebug() << qs;
    }
    qDebug() << "---";
#endif
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QKalmanFilter::refreshMatrixQ(double dTimeDiff) {
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
//  dDuration - relaxaetion time (sec)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QKalmanFilter::matrixRelaxation(double dDuration) {
    refreshMatrixQ(m_dDeltaT);
    Matrix mKalPp= (*m_pPp);
    Matrix mKalPm= mKalPp;
    // Matrix mKalF = (*m_pF);
    Matrix mKalH = (*m_pH);
    Matrix mKalK = (*m_pK);
    Matrix mKalR = (*m_pR);
    Matrix mKalQ = (*m_pQ);

    Matrix mDenominator(mKalK.rowsize(),mKalK.rowsize());
    double dCurT=0.0e0; // time (sec): blind variable
#ifdef DEBUG_MATRXIP
    int i=0;
#endif
    while (dCurT<dDuration) {
        //------------ increment current time ------------
        dCurT += m_dDeltaT;
#ifdef DEBUG_MATRXIP
        bInRelax=true;
#endif
        matrixEvolution(m_dDeltaT);
        //------------ prior covariations ----------------
        mKalPm =(*m_pPm);
        mKalPm = mKalPm+mKalQ;
        //--------------- Kalman gain --------------------
        mDenominator= mKalH * mKalPm * (~mKalH) + mKalR;
        m_dDet=mDenominator.det();
        if (abs(m_dDet)<QKALMANFILTER_EPS) throw RmoException(QString("QVoiProcessor: det=%1").arg(m_dDet));
        mKalK=mKalPm*(~mKalH)*(!mDenominator);
        //--------------- system noise posterior covariation P_{k(+)} ------------------
        double dIdentity[]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        Matrix mBracket(4,4,dIdentity);
        mBracket= mBracket-mKalK*mKalH;
        mKalPp = mBracket * mKalPm * (~mBracket)
            + mKalK * mKalR * (~mKalK);
        (*m_pPp)= mKalPp;
#ifdef DEBUG_MATRXIP
        if (i++%1000==0) {
            qDebug() << i << " -- Matrix mKalPp";
            for (int i=0; i<mKalPp.rowno(); i++) {
                QString qs;
                for (int j=0; j<mKalPp.colno(); j++) {
                    qs += QString::number(mKalPp[i][j]) + "   ,   ";
                }
                qDebug() << qs;
            }
        }
#endif
    }
    //------------ save current covariation --------------
    (*m_pPm)= mKalPm;
    (*m_pPp)= mKalPp;
    (*m_pK) = mKalK;
#ifdef DEBUG_MATRXIP
    qDebug() << "m_dDeltaT=" << m_dDeltaT << " m_dIntegrStep=" << m_dIntegrStep << " dDuration=" << dDuration;
    qDebug() << "Matrix mKalQ";
    for (int i=0; i<mKalQ.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalQ.colno(); j++) {
            qs += QString::number(mKalQ[i][j]) + "   ,   ";
        }
        qDebug() << qs;
    }
    qDebug() << "Matrix mKalH";
    for (int i=0; i<mKalH.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalH.colno(); j++) {
            qs += QString::number(mKalH[i][j]) + "   ,   ";
        }
        qDebug() << qs;
    }
    qDebug() << "Matrix mKalK";
    for (int i=0; i<mKalK.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalK.colno(); j++) {
            qs += QString::number(mKalK[i][j]) + "   ,   ";
        }
        qDebug() << qs;
    }
    qDebug() << "Matrix mKalPp";
    for (int i=0; i<mKalPp.rowno(); i++) {
        QString qs;
        for (int j=0; j<mKalPp.colno(); j++) {
            qs += QString::number(mKalPp[i][j]) + "   ,   ";
        }
        qDebug() << qs;
    }
#endif

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  dDuration - evolution time (sec)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QKalmanFilter::matrixEvolution(double dDuration) {
    Matrix mKalF = (*m_pF);
    Matrix mKalPp= (*m_pPp);
    Matrix mKalPm= mKalPp;
    Matrix mIncrement(4,4);
    double dCurT=0.0e0; // time (sec): blind variable
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
#ifdef DEBUG_MATRXIP
    if (!bInRelax) {
        qDebug() << "m_dIntegrStep=" << m_dIntegrStep << " dDuration=" << dDuration;
        qDebug() << "Matrix mKalPm";
        for (int i=0; i<mKalPm.rowno(); i++) {
            QString qs;
            for (int j=0; j<mKalPm.colno(); j++) {
                qs += QString::number(mKalPm[i][j]) + "   ,   ";
            }
            qDebug() << qs;
        }
    }
#endif
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QKalmanFilter::assign_mZ(PPOITE pPoite) {
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
// iTk (msec), dMaxLatOffset (m)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QKalmanFilter::spaceStrob(qint64 iTk, PPOITE pPoite, double &dMaxLatOffset, bool &bTimeout) {

    //------------------ preliminary checks of POITE and time ----------------
    // same-strob match discarded
    if (iTk<=m_iTlast) {
        return false;
    }
    // test for trajectory timeout
    bTimeout=false;
    if (iTk>m_iTlast+m_iTimeout) {
        bTimeout=true;
        return false;
    }

    // verify POITE:: match count
    if (pPoite->Count<0 || pPoite->Count>3) {
        throw RmoException("pPoite->Count out of range!");
        return false;
    }

    //------------------------- S-Mode data matching --------------------------
    if (m_bStickTrajToModeS) { // trajectory sticks to it's SmodeCall
        if (!m_qsSmodeCall.isEmpty()) { // match m_qsSmodeCall if initialized
            if(pPoite->bSmodeCall[0] == '\0') { // SmodeCall explicitly established => check it against traj SmodeCall
                QByteArray ba(&pPoite->bSmodeCall[0], 8);
                if (m_qsSmodeCall != QString(ba)) return false; // check failed => spaceStrob returns false
            }
        }
        else if (m_iSmodeAdr != -1) { // match m_iSmodeAdr only if m_qsSmodeCall is empty and m_iSmodeAdr is initialized
            if(pPoite->iSmodeAdr != -1) { // m_iSmodeAdr explicitly established => check it against traj m_iSmodeAdr
                if(pPoite->iSmodeAdr != m_iSmodeAdr) return false; // check failed => spaceStrob returns false
            }
        }
        else if (!m_qsModeSData.isEmpty()) { // match m_qsModeSData only if m_iSmodeAdr is (-1)
                                               // m_qsSmodeCall is empty and m_qsModeSData is initialized
            if(pPoite->rx[0].bModeSData[0] == '\0') { // ModeSData explicitly established => check it against traj ModeSData
                QByteArray ba((char*)&pPoite->rx[0].bModeSData[0],sizeof(pPoite->rx[0].bModeSData));
                if (m_qsModeSData != QString(ba.toBase64().constData())) return false; // check failed => spaceStrob returns false
            }
        }
    }

    //------------------------- space strob evaluation --------------------------
    // maximum tangential offset corresponding to path diff mismatch
    dMaxLatOffset=-1.0e0;

    // time increment
    m_dDeltaT=(iTk-m_iTlast)*1.0e-3; // seconds
    if (m_dDeltaT>10.0e0*60*60 || m_dDeltaT<=0.0e0) {
        throw RmoException(QString("QKalmanFilter::filterStep m_dDeltaT=%1 sec").arg(m_dDeltaT));
        return false;
    }

    //---------------------------- auxilary geodetic vars --------------------------
    refreshGeodeticParams();

    // max spread (m) in topocentric plane
    double dStrobSpreadMin=m_dStrobSpreadMin*1.0e3; // min strob (m)
    double dStrobSpread=dStrobSpreadMin+m_dStrobSpreadSpeed*m_dDeltaT;

    // normal direction in geocentric Cartesian coordinates
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

    // xyzTg -- geocentric coordinates of target
    blhTg.dLat = mXm[0][0]*RAD_TO_DEG;
    blhTg.dLon = mXm[2][0]*RAD_TO_DEG;
    blhTg.dHei = m_dHei;
    m_geoUtils.BlhToXyz(&blhTg,&xyzTg);
    if (qIsNaN(xyzTg.dX) || qIsNaN(xyzTg.dY) || qIsNaN(xyzTg.dZ)) {
        throw RmoException(QString("QKalmanFilter: qIsNaN(xyzTg.dX,Y,Z): Lat=%1 Lon=%2"
                                   "\nmXp (%3, %4, %5, %6)"
                                   "\nmXprimeDt (%7, %8, %9, %10)"
                                   "\ndeltaT=%11 m_dKalf1=%12 m_dKalf2=%13")
                           .arg(blhTg.dLat).arg(blhTg.dLon)
                           .arg(mXp[0][0]).arg(mXp[1][0]).arg(mXp[2][0]).arg(mXp[3][0])
                           .arg(mXprimeDt[0][0]).arg(mXprimeDt[1][0]).arg(mXprimeDt[2][0]).arg(mXprimeDt[3][0])
                           .arg(m_dDeltaT).arg(m_dKalf1).arg(m_dKalf3)
                           );
        return false;
    }

    if (pPoite->Count!=1 && pPoite->Count!=2 && pPoite->Count!=3) {
        throw RmoException(QString("QKalmanFilter: bad pPoite->Count=%1")
                .arg(pPoite->Count));
        return false;
    }

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

        if (qIsNaN(dGradX) || qIsNaN(dGradY) || qIsNaN(dGradZ)) {
            throw RmoException(QString("QKalmanFilter: qIsNaN(dGradX,Y,Z)\nTg: (%1,%2,%3)\ndR=%4 dR1=%5"
                                       "\nxyzPost: (%6,%7,%8)"
                                       "\nxyzPost1: (%9,%10,%11)")
                               .arg(xyzTg.dX).arg(xyzTg.dY).arg(xyzTg.dZ)
                               .arg(dR).arg(dR1)
                               .arg(xyzPost.dX).arg(xyzPost.dY).arg(xyzPost.dZ)
                               .arg(xyzPost1.dX).arg(xyzPost1.dY).arg(xyzPost1.dZ));
            return false;
        }

        // magnitude of topocentric lateral (tangent) projection of measurement gradient
        double dTanGrad;
        xx=dGradX*(1.0e0-dXn); yy=dGradY*(1.0e0-dYn); zz=dGradZ*(1.0e0-dZn);
        dTanGrad=sqrt(xx*xx+yy*yy+zz*zz);
        if (dTanGrad<QKALMANFILTER_EPS) {
            throw RmoException("dTanGrad<QKALMANFILTER_EPS");
            return false;
        }
        if (0) qDebug() << QString("QKalmanFilter: dTanGrad=%1\n"
                                           "Grad x,y,z=%2, %3, %4\n"
                                           "Normal x,y,z=%5, %6, %7")
                                           .arg(dTanGrad)
                                           .arg(dGradX).arg(dGradY).arg(dGradZ)
                                           .arg(dXn).arg(dYn).arg(dZn);
        // spread of path difference measurement
        double dPathDiffSpread=dTanGrad*dStrobSpread;
        double dPathDiff=dR-dR1;
        double dPathDiffOffset=abs(dPathDiff-pPoite->rx[iMatch].D);
        if (dPathDiffOffset>dPathDiffSpread) {
            if (0) qDebug() << "dDeltaT,iMatch,dTanGrad,dPathDiffOffset,dPathDiffSpread="
                     << " " << m_dDeltaT
                     << " " << iMatch
                     << " " << dTanGrad
                     << " " << dPathDiffOffset
                     << " " << dPathDiffSpread;
            return false;
        }

        // transform path diff mismatch to tangential offset
        double dLatOffset=dPathDiffOffset/dTanGrad;
        if (dLatOffset<=0.0e0 || qIsNaN(dLatOffset)) {
            throw RmoException(QString("QKalmanFilter: dLatOffset<=0.0e0. dPathDiffOffset=%1; dTanGrad=%2\n"
                                   "Grad x,y,z=%3, %4, %5\n"
                                   "Normal x,y,z=%6, %7, %8")
                                   .arg(dPathDiffOffset).arg(dTanGrad)
                                   .arg(dGradX).arg(dGradY).arg(dGradZ)
                                   .arg(dXn).arg(dYn).arg(dZn)
                                                  );
            return false;
        }
        if (dMaxLatOffset<dLatOffset) dMaxLatOffset=dLatOffset;
        if (dMaxLatOffset<0.0e0) qDebug() << QString("dLatOffset: %1 = %2 / %3").arg(dLatOffset).arg(dPathDiffOffset).arg(dTanGrad);
    }
    if (dMaxLatOffset<0.0e0) {
        throw RmoException(QString("QKalmanFilter: negative dMaxLatOffset=%1").arg(dMaxLatOffset));
        return false;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   iTk -- current time (msec)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QKalmanFilter::filterStep(qint64 iTk, PPOITE pPoite, bool &bOverrun, bool &bTimeout) {

    bTimeout=false; bOverrun=false;
    //---------------------------- check for timeout ---------------------------------
    if (iTk > m_iTlast + m_iTimeout) {
        bTimeout=true;
        return false;
    }
    //---------------------------- time interval evaluation --------------------------
    if (iTk<=m_iTlast) {
        throw RmoException(QString("QKalmanFilter::filterStep iTk(%2)<=m_iTlast(%1) iTk-iTlast=%3")
                           .arg(m_iTlast).arg(iTk).arg(iTk-m_iTlast));
        return false;
    }
    m_dDeltaT=(iTk-m_iTlast)*1.0e-3; // seconds
    if (m_dDeltaT>10.0e0*60*60 || m_dDeltaT<=0.0e0) {
        throw RmoException(QString("QKalmanFilter::filterStep m_dDeltaT=%1 sec").arg(m_dDeltaT));
        return false;
    }

    //---------------------------- auxilary geodetic vars --------------------------
    refreshGeodeticParams();

    //---------------------------- system force matrix --------------------------
    if (!m_bFinitialized) {
        // m_bFinitialized=true;
        refreshMatrixF();
    }

    //---------------------------- system noise covariation --------------------------
    if (!m_bQinitialized) {
        // m_bQinitialized=true;
        refreshMatrixQ(m_dDeltaT);
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
    }

    //--------------- system noise covariation P_{k(-)} evolution ------------------
    if (m_bUseMatRelax && !m_bMatRelaxed) {
        refreshMatrixK(pPoite);
        matrixRelaxation(m_dMatRelaxTime);
        m_bMatRelaxed=true;
    }
#ifdef DEBUG_MATRXIP
    bInRelax=false;
#endif
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

    //------------ save the posterior state estimate \hat{x}_{k(+)} ------------
    (*m_pXp) = mXp;
    if (qIsNaN(mXp[0][0]) || qIsNaN(mXp[2][0])) {
        using std::cout;
        Matrix mdZ=mZ-mZm;
        throw RmoException(QString("filterStep: qIsNan(mXp[0][0]) || qIsNan(mXp[2][0])\n"
                           "mXp=(%1, %2, %3, %4)\n"
                           "mXm=(%5, %6, %7, %8)\n"
                           "mMismatchCorr=(%9, %10, %11, %12)\n"
                           "mdZ=(%13, %14, %15, %16)"
                           "mKalK=(%17, %18, %19, %20\n%21, %22, %23, %24\n %25, %26, %27, %28\n %29, %30, %31, %32)")
                .arg(mXp[0][0]).arg(mXp[1][0]).arg(mXp[2][0]).arg(mXp[3][0])
                .arg(mXm[0][0]).arg(mXm[1][0]).arg(mXm[2][0]).arg(mXm[3][0])
                .arg(mMismatchCorr[0][0]).arg(mMismatchCorr[1][0]).arg(mMismatchCorr[2][0]).arg(mMismatchCorr[3][0])
                .arg(mdZ[0][0]).arg(mdZ[1][0]).arg(mdZ[2][0]).arg(mdZ[3][0])
                .arg(mKalK[0][0]).arg(mKalK[0][1]).arg(mKalK[0][2]).arg(mKalK[0][3])
                .arg(mKalK[1][0]).arg(mKalK[1][1]).arg(mKalK[1][2]).arg(mKalK[1][3])
                .arg(mKalK[2][0]).arg(mKalK[2][1]).arg(mKalK[2][2]).arg(mKalK[2][3])
                .arg(mKalK[3][0]).arg(mKalK[3][1]).arg(mKalK[3][2]).arg(mKalK[3][3])
                );
        return false;
    }

    //------------------- update current system time  -----------------------
    m_iTlast=iTk;

    //------------------- check for overrun ---------------------------------
    double vxx=mXp[1][0]; vxx=vxx*vxx;
    double vyy=mXp[3][0]; vyy=vyy*vyy;
    double vabs=sqrt(vxx+vyy); // absolute velocity (m/s)
    if (vabs > m_dTrajMaxVelocity) {
        bOverrun=true;
        return false;
    }

    //------------------- assign SmodeCall ----------------------------------
    if (m_qsSmodeAdr.isEmpty() && pPoite->iSmodeAdr != -1) {
        m_iSmodeAdr=pPoite->iSmodeAdr;
        m_qsSmodeAdr.sprintf("%06lX", m_iSmodeAdr);
    }
    if (m_qsSmodeCall.isEmpty() && pPoite->bSmodeCall[0] != '\0') {
        QByteArray ba(&pPoite->bSmodeCall[0], 8);
        m_qsSmodeCall = QString(ba);
    }
    if (m_qsModeSData.isEmpty() && pPoite->rx[0].bModeSData[0] != '\0' ) {
        QByteArray ba((char*)&pPoite->rx[0].bModeSData[0],sizeof(pPoite->rx[0].bModeSData));
        m_qsModeSData=QString(ba.toBase64().constData());
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
    if (m_pR) {
        delete m_pR;
        m_pR = NULL;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
XYZ QKalmanFilter::getTgXYZ() {
    XYZ xyzFlt;
    BLH blhFlt;
    // posterior state estimate \hat{x}_{k(+)}
    if (!m_pXp) throw RmoException("QKalmanFilter: m_pXp is NULL");
    Matrix mXp = (*m_pXp);
    blhFlt.dLat=mXp[0][0];
    blhFlt.dLon=mXp[2][0];
    blhFlt.dHei=m_dHei;
    m_geoUtils.toTopocentric(&m_blhViewPoint,&blhFlt,&xyzFlt);
    return xyzFlt;
}
