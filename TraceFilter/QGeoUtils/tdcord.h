#ifndef TDCORD_H
#define TDCORD_H

#include <QtCore>

#include "qavtctrl.h"
#include "codograms.h"

#include "geocent.h"

#define KRASOVSKY_MAJOR_SEMIAXIS 6378245
#define KRASOVSKY_MINOR_SEMIAXIS 6356863

//******************************************************************************
//
//******************************************************************************
class TdCord
{
public:
    TdCord() { }
    static void Blh2Xyz(BLH blh, PXYZ pp);
    static void Xyz2Blh(XYZ xyz, PBLH pp);
    static void toTopocentric(PBLH pblhViewPoint,PBLH pblhTg,PXYZ pxyzTc);

    static GeocentricInfo gi;

private:
    static void toTopocentric(BLH viewPoint, double *gc_loc, double *tc_loc);
};

#endif // TDCORD_H
