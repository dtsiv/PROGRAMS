#ifndef QVOIPROCESSOR_H
#define QVOIPROCESSOR_H
#include "qgeoutils.h"
#include "qpoimodel.h"
#include "tpoit.h"
#include "qgenerator.h"
#include "qkalmanfilter.h"

#include <iostream>

#include "cmatrix"
typedef techsoft::matrix<double> Matrix;

#define TIMESTAMP_FORMAT "dd.MM.yyyy-hh:mm:ss.zzz"

#define VOI_TYPE_PRIMARY 0
#define VOI_TYPE_FLT_OUT 1
#define VOI_TYPE_GEN_SRC 2

#define EST_ERR_COORD    100    // m
#define EST_ERR_SPEED    10     // m/s

#define QVOIPROCESSOR_EPS 1.0e-18
#define QVOIPROCESSOR_CHI2_NUM_QUANTILES 6

class QTraceFilter;
class QPrimaryCluster;

class QVoiProcessor : public QObject 
{
	Q_OBJECT
public:
	QVoiProcessor(QTraceFilter *pOwner, QPoiModel *pPoiModel, QObject *parent=0);
	~QVoiProcessor();

    bool init(QString qsMainctrlCfg);
    void listPosts();
    void receiveCodogram(quint64 iTcur, PPOITE pPoite, BLH *pblhGenSRC=NULL, int iRawIdx=-1);
    void start(quint64 tFrom, quint64 tTo, bool bUseGen);
	void filterStep(qint64 iTk, PPOITE pPoite);

	static bool m_bUseGen; // use gen as data source
	static double m_dTrajDuration; // trajectory duration
	static bool m_pbSkipPost[5]; // posts mask

signals:
    void addPost(double x, double y, int iId);
    void indicatorPoint(double x, double y, int iType, TPoiT* pTPoiT, int iRawIdx);
    void indicatorUpdate();
    void matrixP(QString);

public:
	Matrix *m_pPp;

private:
    void refreshGeodeticParams();
	void assignPosts(PPOITE pPoite);
    QList<int> getActivePosts(PPOITE pPoite);

	double m_dDet;
    double m_dDeltaT;

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

	PMAINCTRL m_pMainCtrl;
	QTraceFilter *m_pOwner;
	QGenerator *m_pGen;
	QPoiModel *m_pPoiModel;
	QGeoUtils m_geoUtils;
    BLH m_blhViewPoint; // dLat,dLon - radians, dHei - meters
	qint64 m_iTstart;
    bool m_bMatRelaxed;
	int m_iLegendTypePrimary;
	int m_iLegendTypeSource;
	int m_iLegendTypeSourceAlarm;
	int m_iLegendTypeFilter;
    int m_iLegendTypeCluster;
    bool m_bInit;
    QList<QKalmanFilter*> m_qlKalmanFilters;
    QList<QPrimaryCluster*> m_qlClusters;

};

class QPrimaryCluster {
public:
    QPrimaryCluster(qint64 iTk, double dTgX, double dTgY);
    bool isStale(qint64 iTk);
    bool appendPriPt(qint64 iTk, double dTgX, double dTgY);
    double distance(double dTgX, double dTgY);
    void getVelocityEst(double &dVx,double &dVy);
    XYPOINT massCenter() {
        XYPOINT retVal;
        retVal.dX=m_dCenterX; retVal.dY=m_dCenterY;
        return retVal;
    }

private:
    QList<qint64> m_qlTk;
    QList<double> m_qlTgX;
    QList<double> m_qlTgY;
    double m_dCenterX,m_dCenterY;
    int m_iSize;
    qint64 m_iStart; // msec
    qint64 m_iTimeout; // msec
};

#endif // QVOIPROCESSOR_H
