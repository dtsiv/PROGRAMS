#ifndef TDCORD_H
#define TDCORD_H

#include <QtCore>
#include <QList>

#include "qavtctrl.h"
#include "codograms.h"

#include "geocent.h"

#define KRASOVSKY_MAJOR_SEMIAXIS 6378245
#define KRASOVSKY_MINOR_SEMIAXIS 6356863

#define RAD_TO_DEG	57.29577951308232
#define DEG_TO_RAD	0.0174532925199432958

#define MAX_dwPosCount 8

//******************************************************************************
//
//******************************************************************************
class TdCord
{
public:
    struct GeodeticToTopocentricCache {
        double h; // pblhViewPoint->dHei;
        double sinp; // sin(phi)
        double cosp; // cos(phi), phi=pblhViewPoint->dLat;
        double sinl; // sin(lam)
        double cosl; // cos(lam), lam=pblhViewPoint->dLon;
        double axi;  // gi.Geocent_a/sqrt(1.0e0 - e2*sinp*sinp)
        double dz;   // axi*e2*sinp;
    };

    TdCord() { }
    static void Blh2Xyz(BLH blh, PXYZ pp);
    static void Xyz2Blh(XYZ xyz, PBLH pp);
    static void initGeodeticToTopocentricCache(PBLH pblhViewPoint,struct GeodeticToTopocentricCache *pCache);
    static void toTopocentric(PBLH pblhViewPoint,BLH blhGeodetic,PXYZ pxyzTc, struct GeodeticToTopocentricCache *pCache = 0);
    static void fromTopocentric(PBLH pblhViewPoint,XYZ xyzTc,PBLH pblhGeodetic,struct GeodeticToTopocentricCache *pCache = 0);

    static bool getViewPoint(PBLH pblhViewPoint,PMAINCTRL pMainCtrl);
    static bool getTopocentricPostsList(PBLH pblhViewPoint,QList<XYZ> &qlTcPosts,QList<int> &qlPostIds,PMAINCTRL pMainCtrl);

    static GeocentricInfo gi;
};

#endif // TDCORD_H
