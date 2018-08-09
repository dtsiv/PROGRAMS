#include "qkalmanfilter.h"
#include "rmoexception.h"

QKalmanFilter::QKalmanFilter(const QGeoUtils &geoUtils, double dHei) :
       m_geoUtils(geoUtils), m_dHei(dHei) {
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


