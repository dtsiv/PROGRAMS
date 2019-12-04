#include "tdcord.h"
#include "qexceptiondialog.h"

// initialization using KRASOVSKY_MAJOR_SEMIAXIS and KRASOVSKY_MINOR_SEMIAXIS
GeocentricInfo TdCord::gi = {
    // double Geocent_a;        /* Semi-major axis of ellipsoid in meters */
    6378245.e0,
    // double Geocent_b;        /* Semi-minor axis of ellipsoid           */
    6356863.e0,
    // double Geocent_a2;       /* Square of semi-major axis */
    40682009280025.e0,
    // double Geocent_b2;       /* Square of semi-minor axis */
    40409707200769,
    // double Geocent_e2;       /* Eccentricity squared  */
    0.0066934274898192208311242801244134170702,
    // double Geocent_ep2;      /* 2nd eccentricity squared */
    0.0067385313608710846119964621436793095199
};

//******************************************************************************
// blh: radians, meters; pp: meters
//******************************************************************************
void TdCord::Blh2Xyz(BLH blh, PXYZ pp) {
    long retVal = pj_Convert_Geodetic_To_Geocentric (
        &gi, blh.dLat, blh.dLon, blh.dHei,
        &pp->dX, &pp->dY, &pp->dZ);
    switch (retVal) {
        case GEOCENT_NO_ERROR: return;
        case GEOCENT_LAT_ERROR:
            throw RmoException("Blh2Xyz: Latitude out of valid range (-90 to 90 degrees)");
            return;
        case GEOCENT_LON_ERROR:
            throw RmoException("Blh2Xyz: Longitude out of valid range (-180 to 360 degrees)");
            return;
        case GEOCENT_A_ERROR:
            throw RmoException("Blh2Xyz: Semi-major axis lessthan or equal to zero");
            return;
        case GEOCENT_B_ERROR:
            throw RmoException("Blh2Xyz: Semi-minor axis lessthan or equal to zero");
            return;
        case GEOCENT_A_LESS_B_ERROR:
            throw RmoException("Blh2Xyz: Semi-major axis less than semi-minor axis");
            return;
        default:
            throw RmoException("Blh2Xyz: pj_Convert_Geodetic_To_Geocentric unknown error");
            return;
    }
}
//******************************************************************************
//
//******************************************************************************
bool TdCord::getViewPoint(PBLH pblhViewPoint,PMAINCTRL pMainCtrl) {
    if (!pblhViewPoint || !pMainCtrl) return false;
    pblhViewPoint->dLat=0.0e0; pblhViewPoint->dLon=0.0e0; pblhViewPoint->dHei=0.0e0;
    if (pMainCtrl->p.dwPosCount<=4 || pMainCtrl->p.dwPosCount>MAX_dwPosCount) return false;
    for (int i=1; i<=4; i++) {
        PGROUNDINFO pgi = &pMainCtrl -> p.positions[i];
        pblhViewPoint->dLat += pgi->blh.dLat*DEG_TO_RAD;
        pblhViewPoint->dLon += pgi->blh.dLon*DEG_TO_RAD;
    }
    pblhViewPoint->dLat/=4.0e0; pblhViewPoint->dLon/=4.0e0;
    return true;
}
//******************************************************************************
//
//******************************************************************************
bool TdCord::getTopocentricPostsList(PBLH pblhViewPoint,QList<XYZ> &qlTcPosts,QList<int> &qlPostIds,PMAINCTRL pMainCtrl) {
    if (!pblhViewPoint || !pMainCtrl) return false;
    if (pMainCtrl->p.dwPosCount<=4 || pMainCtrl->p.dwPosCount>MAX_dwPosCount) return false;
    qlTcPosts.clear();
    for (int i=1; i<=4; i++) {
        PGROUNDINFO pgi = &pMainCtrl -> p.positions[i];
        XYZ postXYZ;
        BLH postBLH;
        postBLH.dLat=pgi->blh.dLat*DEG_TO_RAD; postBLH.dLon=pgi->blh.dLon*DEG_TO_RAD;
        postBLH.dHei=pgi->blh.dHei;
        toTopocentric(pblhViewPoint,postBLH,&postXYZ); // radians,meters
        qlTcPosts << postXYZ;
        qlPostIds << i;
    }
    return true;
}
//******************************************************************************
//   xyz - meters, pp - radians, meters
//******************************************************************************
void TdCord::Xyz2Blh(XYZ xyz, PBLH pp) {
    pj_Convert_Geocentric_To_Geodetic(
        &gi, xyz.dX, xyz.dY, xyz.dZ,
        &pp->dLat, &pp->dLon, &pp->dHei);
}
/*=========================================================
 * PBLH pblhViewPoint,BLH blhGeodetic:
 * dLat,dLon in radians
 * dHei in meters
 * pxyzTc in meters
 *=========================================================*/
void TdCord::toTopocentric(PBLH pblhViewPoint,BLH blhGeodetic,PXYZ pxyzTc) {
    if (!pblhViewPoint) {
        throw RmoException("TdCord::toTopocentric - pblhViewPoint is NULL");
        return;
    }
    XYZ xyzGc;
    Blh2Xyz(blhGeodetic,&xyzGc);

    double phi=pblhViewPoint->dLat;
    double lam=pblhViewPoint->dLon;
    double h  =pblhViewPoint->dHei;
    double sinp = sin(phi), cosp = cos(phi), sinl = sin(lam), cosl = cos(lam);
    double e2   = gi.Geocent_e2;
    double axi = sinp*sinp; axi = 1.0e0 - e2*axi; axi = gi.Geocent_a/sqrt(axi);

    // initialize topocentric coordinates
    pxyzTc->dX = xyzGc.dX;
    pxyzTc->dY = xyzGc.dY;
    double dz=axi*e2*sinp;

    // shift coordinate system down along Oz
    pxyzTc->dZ = xyzGc.dZ + dz;

    // temporary storage of rotated coordinates
    double xr,yr,zr;

    // rotate around Oz by lambda counter-clockwise
    xr = cosl*pxyzTc->dX + sinl*pxyzTc->dY;
    yr =-sinl*pxyzTc->dX + cosl*pxyzTc->dY;
    pxyzTc->dX = xr;
    pxyzTc->dY = yr;

    // rotate around Oy by pi/2-phi counter-clockwise
    xr = sinp*pxyzTc->dX - cosp*pxyzTc->dZ;
    zr = cosp*pxyzTc->dX + sinp*pxyzTc->dZ;
    pxyzTc->dX = xr;
    pxyzTc->dZ = zr;

    // shift along Oz by a*xi+h
    dz = axi + h;
    pxyzTc->dZ = pxyzTc->dZ - dz;

    // direct x to the North
    pxyzTc->dX = -pxyzTc->dX;

   // swap x and y
   double tmp=pxyzTc->dX; pxyzTc->dX=pxyzTc->dY; pxyzTc->dY=tmp;
   return;
}
