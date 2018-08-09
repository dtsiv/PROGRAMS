#ifndef QGEOUTILS_H
#define QGEOUTILS_H

#include <QObject>
#include <QFile>

#include <iostream>
#include <math.h>

#include "qavtctrl.h"
#include "codograms.h"

class QGeoUtils : public QObject
{
public:
	QGeoUtils(QObject *parent=0);
    QGeoUtils(const QGeoUtils &guTemplate, QObject *parent=0);
	~QGeoUtils();

	bool readCfgFile(QString qsMainctrlCfg);

    bool isValidConfig(int iIdlePost,bool bP[]);
    void BlhToXyz(double dLat, double dLon, double dHei,
				  double * pdX, double * pdY, double * pdZ);
    void BlhToXyz(PBLH pblh, PXYZ pxyz);
    void XyzToBlh(double dX, double dY, double dZ,
                         double * pdLat, double * pdLon, double * pdHei);
    void XyzToBlh(PXYZ pxyz, PBLH pblh);
    double determinant3x3(double mat[9]);
    void toTopocentric(PBLH pblhViewPoint,PBLH pblhTg,PXYZ pxyzTc);
    void toTopocentric(BLH viewPoint,double *gc,double *tc);
	void getViewPoint(PBLH pblhViewPoint,PMAINCTRL pMainCtrl);

    const int NUMBER_OF_POSTS;
    const int NPAIRS;

	const double RAD_2_DEG;
    const double DEG_2_RAD;

    const double DEFMAJOR;
    const double DEFMINOR;

    const double EllipsMajor;
    const double EllipsMinor;

    // light speed in m/ns
    const double SPEED_OF_LIGHT;
    const double MISMATCH_THRESH;
    const double EPS;

	QFile m_fMainctrlCfg;
    PMAINCTRL m_pMainCtrl;
    double *tc, *gc;
	BLH m_blhPosts[5];
};

#endif // QGEOUTILS_H
