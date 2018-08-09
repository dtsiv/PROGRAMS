#ifndef QGENERATOR_H
#define QGENERATOR_H

#include <QtCore>
#include <QSettings>

#include "qgeoutils.h"
#include "tpoit.h"

// for gasdev
#include <string>
#include <iostream>
#include <iomanip>
#include "nr.h"
using namespace std;

// #define QGENERATOR_USE_HEIGHT_CORRECTION
#define QGENERATOR_DUMMY_DEFAULT    -999999.99e99

class QGenerator
{
public:
	enum TrajectoryTypes {
		LinearTrajectory,
		LinearWithKink,
		CircleTrajectory
	};

	QGenerator(QGeoUtils * pGeoUtils,qint64 iTime,PBLH pblhViewPoint);

	~QGenerator();

    void resetTime(qint64 iTime);
    void resetTarget(PBLH pblhTg);
    void setTargetHeight(double dHei);
    void moveTarget(double dX, double dY);
    PBLH getTg();
    void getTgVel(double &vx, double &vy);

    bool propagate(qint64 iTime);
    PPOITE getPoite(bool bIdlePosts[]);
    void resetRandomNumberGenerators();

	qint64      m_iTstart;

	static double m_dGenDelay; // delay s
	static double m_dGenX0; // gen init x0
	static double m_dGenY0; // gen init y0
	static double m_dGenV0; // gen init v
	static double m_dGenAz; // gen init Az
	static double m_dGenTrajR; // gen traj R
	static double m_bGenTrajCW; // gen traj CW
	static double m_dGenSigmaM; // Poite generator: measurement sigma m
	static double m_dGenHeight; // gen height km
	static double m_dGenKinkTime; // gen kink time min
	static double m_dGenKinkAngle; // gen kink angle deg
	static TrajectoryTypes m_ttGenTrajectory; // type of generator trajectory

	// Gaussian random number seed
	static int m_idum;

private:

	qint64      m_iTime;               // mscec since Unix epoch
	QGeoUtils * m_pGeoUtils;
	BLH         m_blhViewPoint;        // view point 
	BLH         m_blhTg;               // dLat, dLon - radians; dHei - meters
	double      m_dW1, m_dW2;
	double      m_dMajor; 
    double      m_dESq;
	double      m_dTrajR;              // traj radius km
	bool        m_bTrajCW;             // clock wise
	bool        m_bKinkPassed;
};

#endif // QGENERATOR_H
