#ifndef QKALMANFILTER_H
#define QKALMANFILTER_H

#include "qgeoutils.h"
#include "qpoimodel.h"
#include "tpoit.h"

#include <iostream>

#include "cmatrix"
typedef techsoft::matrix<double> Matrix;

#define QKALMANFILTER_EPS 1.0e-18
#define QKALMANFILTER_CHI2_NUM_QUANTILES 6

class QKalmanFilter
{
public:
    QKalmanFilter(qint64 iTk, const QGeoUtils &geoUtils, BLH blhTg, double dVx=0.0e0, double dVy=0.0e0);
	~QKalmanFilter(void);
	void refreshGeodeticParams();
    void refreshMatrixF();
    void refreshMatrixR(PPOITE pPoite);
    void refreshMatrixH(PPOITE pPoite);
    void refreshMatrixK(PPOITE pPoite);
    void refreshMatrixQ(double dTimeDiff); // dTimeDiff (sec)
    void matrixRelaxation(double dDuration); // dDuration (sec)
    void matrixEvolution(double dDuration); // dDuration (sec)
    void assign_mZ(PPOITE pPoite);
    bool spaceStrob(qint64 iTk, PPOITE pPoite, double &dMaxLatOffset, bool &bTimeout);
    bool filterStep(qint64 iTk, PPOITE pPoite, bool &bOverrun, bool &bTimeout);
    XYZ getTgXYZ();

    static double m_dFltSigmaS2; // system noise sigma2
    static double m_dFltSigmaM; // measurement sigma
    static double m_dFltHeight; // filter height km
    static double m_dIntegrStep; // time step (s) for fine-grained matrix evolution
    static double m_dStrobSpreadSpeed; // side spread speed m/s
    static double m_dStrobSpreadMin; // min spread km
    static int m_iChi2Prob; // chi2 probability threshold
    static const int CHI2_NUM_QUANTILES;
    static double m_dChi2QuantProb[QKALMANFILTER_CHI2_NUM_QUANTILES];
    static double m_dChi2Quant[4][QKALMANFILTER_CHI2_NUM_QUANTILES];
    static double m_dMatRelaxTime; // seconds
    static bool m_bUseMatRelax;
    static bool m_bStickTrajToModeS;
    static double m_dClusterCutoff; // cutoff radius for cluster (km)
    static int m_iClusterMinSize;
    static double m_dTrajMaxVelocity; // max velocity (m/s) for trajectory
    static double m_dTrajTimeout;  // trajectory timeout (min)
    static bool m_bEstIniVelocity;

private:
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

    double m_dDet;
    double m_dDeltaT; // time step (sec)

    // matrix pointers
    Matrix *m_pPm;
    Matrix *m_pPp;
    Matrix *m_pQ;
    bool m_bQinitialized;
    Matrix *m_pR;
    Matrix *m_pF;
    bool m_bFinitialized;
    Matrix *m_pK;
    Matrix *m_pH;
    bool m_bHinitialized;

    // posterior state estimate \hat{x}_{k-1(+)}
    Matrix *m_pXp;
    // prior state estimate \hat{x}_{k(-)}
    Matrix *m_pXm;
    // real measurement
    Matrix *m_pZ;
    // posterior measurement estimate \hat{z}_{k-1(+)}
    Matrix *m_pZm;
    QGeoUtils m_geoUtils;
    PMAINCTRL m_pMainCtrl;
    BLH m_blhViewPoint; // dLat,dLon - radians, dHei - meters
    double m_dHei; // filter hight m
    qint64 m_iTlast; // previous time point (msec)
    qint64 m_iTStart; // starting time point (msec)
    qint64 m_iTimeout; // trajectory timeout (msec)
    bool m_bMatRelaxed; // flag: cov matrix was initially relaxed
    int m_iSmodeAdr;    // same as ETVOI::iSmodeAdr
    QString m_qsSmodeAdr;   // same as TTrace::m_qsSmodeAdr
    QString m_qsSmodeCall;  // same as TTrace::m_qsSmodeCall
    QString m_qsModeSData;  // raw SMode data
};

#endif // QKALMANFILTER_H
