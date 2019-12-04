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
    TdCord() { }
    static void Blh2Xyz(BLH blh, PXYZ pp);
    static void Xyz2Blh(XYZ xyz, PBLH pp);
    static void toTopocentric(PBLH pblhViewPoint,BLH blhGeodetic,PXYZ pxyzTc);

    static bool getViewPoint(PBLH pblhViewPoint,PMAINCTRL pMainCtrl);
    static bool getTopocentricPostsList(PBLH pblhViewPoint,QList<XYZ> &qlTcPosts,QList<int> &qlPostIds,PMAINCTRL pMainCtrl);

    static GeocentricInfo gi;
};

#endif // TDCORD_H
