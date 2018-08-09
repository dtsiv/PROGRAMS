#ifndef QKALMANFILTER_H
#define QKALMANFILTER_H

#include "qgeoutils.h"

#include "cmatrix"
typedef techsoft::matrix<double> Matrix;

class QKalmanFilter
{
    // geodetic vars
	double m_dGeoE, m_dGeoE2, m_dGeoEPrime;
	double m_dGeoXi, m_dGeoXi3, m_dGeoXi5, m_dGeoXiPrime;
	double m_dGeoG, m_dGeoG2, m_dGeoGPrime;
	double m_dGeoEps, m_dGeoEps2;
	double m_dGeoSinB, m_dGeoCosB, m_dGeoSin2B, m_dGeoTgB;
	double m_dGeoSinL, m_dGeoCosL;
	double m_dLat, m_dLon;
	double m_dTgW1, m_dTgW2;
	double m_dKalf1, m_dKalf3;

	// matrix pointers
	Matrix *m_pPm;
	Matrix *m_pPp;
	Matrix *m_pQ;
	Matrix *m_pR;
	Matrix *m_pF;
	Matrix *m_pK;
	Matrix *m_pH;

	// posterior state estimate \hat{x}_{k-1(+)}
	Matrix *m_pXp;
	// prior state estimate \hat{x}_{k(-)}
	Matrix *m_pXm;
	// real measurement
	Matrix *m_pZ;
	// posterior measurement estimate \hat{z}_{k-1(+)}
	Matrix *m_pZm;
	QGeoUtils m_geoUtils;
    BLH m_blhViewPoint; // dLat,dLon - radians, dHei - meters
	double m_dHei; // filter hight m

public:
	QKalmanFilter(const QGeoUtils &geoUtils, double dHei);
	~QKalmanFilter(void);
	void refreshGeodeticParams();
};

#endif // QKALMANFILTER_H
