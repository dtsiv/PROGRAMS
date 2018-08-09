#ifndef QVOIPROCESSOR_H
#define QVOIPROCESSOR_H
#include "qgeoutils.h"
#include "qpoimodel.h"
#include "tpoit.h"
#include "qgenerator.h"

#include <iostream>

#include "cmatrix"
typedef techsoft::matrix<double> Matrix;

#define TIMESTAMP_FORMAT "dd.MM.yyyy-hh:mm:ss.zzz"

#define VOI_TYPE_PRIMARY 0
#define VOI_TYPE_FLT_OUT 1
#define VOI_TYPE_GEN_SRC 2

#define EST_ERR_COORD    100    // m
#define EST_ERR_SPEED    10     // m/s

#define CHI2_NUM_QUANTILES 6

#define QVOIPROCESSOR_EPS 1.0e-18

class QTraceFilter;

class QVoiProcessor : public QObject 
{
	Q_OBJECT
public:
	QVoiProcessor(QTraceFilter *pOwner, QPoiModel *pPoiModel, QObject *parent=0);
	~QVoiProcessor();

    bool init(QString qsMainctrlCfg);
    void listPosts();
    void startSimulation(quint64 tFrom, quint64 tTo);
    void startImitator(quint64 tFrom, quint64 tTo);
	void filterStep(qint64 iTk, PPOITE pPoite);

	static double m_dFltSigmaS2; // system noise sigma2
	static double m_dFltSigmaM; // measurement sigma
	static double m_dFltHeight; // filter height km
	static bool m_bUseGen; // use gen as data source
	static double m_dTrajDuration; // trajectory duration
	static bool m_pbSkipPost[5]; // posts mask
	static double m_dIntegrStep; // time step for fine-grained matrix evolution
	static double m_dStrobSpreadSpeed; // side spread speed m/s
	static double m_dStrobSpreadMin; // min spread km
	static int m_iChi2Prob; // chi2 probability threshold
    static double m_dChi2QuantProb[CHI2_NUM_QUANTILES];
    static double m_dMatRelaxTime; // seconds
    static bool m_bUseMatRelax;

signals:
    void addPost(double x, double y, int iId);
    void indicatorPoint(double x, double y, int iType, TPoiT* pTPoiT);
    void indicatorUpdate();
    void matrixP(QString);

public:
	Matrix *m_pPp;

private:
    void refreshGeodeticParams();
    void refreshMatrixF();
    void refreshMatrixQ(double dTimeDiff);
	void refreshMatrixR(PPOITE pPoite);
	void refreshMatrixK(PPOITE pPoite);
	void refreshMatrixH(PPOITE pPoite);
	void refreshMatrixHnum(PPOITE pPoite);
	void assignPosts(PPOITE pPoite);
	void assign_mZ(PPOITE pPoite);
    bool spaceStrob(qint64 iTk, PPOITE pPoite);
	void matrixDebugOutput(bool bAppend, int iTk = 0);
	void matrixEvolution(double dDuration);
    void matrixRelaxation(double dDuration);
    QList<int> getActivePosts(PPOITE pPoite);
    void addItem(QFile &qf, double dVal, bool bIndent, bool bNewline, int iPrecision=3);
    void exactStatistics();

	// covariations of state estimate error
	void covEigenvectors(int iTk =0 );
	QList<double> m_qlErrB,m_qlErrL;
	QList<double> m_qlZ[3];

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

	Matrix *m_pPm;
	Matrix *m_pQ;
	bool m_bQinitialized;
	bool m_bFinitialized;
	bool m_bHinitialized;
	Matrix *m_pR;
	Matrix *m_pF;
	Matrix *m_pK;
	Matrix *m_pH;
	// measurement function nonlinearity
	double m_dMeasNonlinearity;
	Matrix *m_pHnum;
	// posterior state estimate \hat{x}_{k-1(+)}
	Matrix *m_pXp;
	// prior state estimate \hat{x}_{k(-)}
	Matrix *m_pXm;
	// real measurement
	Matrix *m_pZ;
	// posterior measurement estimate \hat{z}_{k-1(+)}
	Matrix *m_pZm;
	PMAINCTRL m_pMainCtrl;
	QTraceFilter *m_pOwner;
	QGenerator *m_pGen;
	QPoiModel *m_pPoiModel;
	QGeoUtils m_geoUtils;
    BLH m_blhViewPoint; // dLat,dLon - radians, dHei - meters
	qint64 m_iTlast;
	qint64 m_iTstart;
    bool m_bMatRelaxed; // seconds
	double m_dHei; // filter hight m
	int m_iLegendTypePrimary;
	int m_iLegendTypeSource;
	int m_iLegendTypeSourceAlarm;
	int m_iLegendTypeFilter;
	bool m_bInit;
	int m_iPt;
};

#endif // QVOIPROCESSOR_H
