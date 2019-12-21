#include "tdcord.h"
#include "qexceptiondialog.h"
#include "poitunit.h"

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
//   xyz - meters, pp - radians, meters
//******************************************************************************
void TdCord::Xyz2Blh(XYZ xyz, PBLH pp) {
    pj_Convert_Geocentric_To_Geodetic(
        &gi, xyz.dX, xyz.dY, xyz.dZ,
        &pp->dLat, &pp->dLon, &pp->dHei);
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
//
//******************************************************************************
void TdCord::initGeodeticToTopocentricCache(PBLH pblhViewPoint,struct GeodeticToTopocentricCache *pCache) {
    if (!pblhViewPoint || !pCache) {
        throw RmoException("TdCord::initGeodeticToTopocentricCache - pblhViewPoint,pCache is NULL");
        return;
    }
    double phi   = pblhViewPoint->dLat;
    double lam   = pblhViewPoint->dLon;
    pCache->h    = pblhViewPoint->dHei;
    double sinp,cosp,sinl,cosl;
    pCache->sinp = sinp = sin(phi);
    pCache->cosp = cosp = cos(phi);
    pCache->sinl = sinl = sin(lam);
    pCache->cosl = cosl = cos(lam);
    double e2    = gi.Geocent_e2;
    double axi   = sinp*sinp; axi = 1.0e0 - e2*axi;
    pCache->axi  = gi.Geocent_a/sqrt(axi);
    pCache->dz   = axi*e2*sinp;
}
/*=========================================================
 * PBLH pblhViewPoint,BLH blhGeodetic:
 * dLat,dLon in radians
 * dHei in meters
 * pxyzTc in meters
 *=========================================================*/
void TdCord::toTopocentric(PBLH pblhViewPoint,BLH blhGeodetic,PXYZ pxyzTc, struct GeodeticToTopocentricCache *pCache /* = 0 */) {
    // transform coefficients -- may be cached
    double h,sinp,cosp,sinl,cosl,axi,dz;
    if (!pCache) { // no cache provided
        // check view point pointer
        if (!pblhViewPoint) {
            throw RmoException("TdCord::toTopocentric - pblhViewPoint is NULL");
            return;
        }
        // calculate transform coefficients
        double phi=pblhViewPoint->dLat;
        double lam=pblhViewPoint->dLon;
        h  =pblhViewPoint->dHei;
        sinp = sin(phi), cosp = cos(phi), sinl = sin(lam), cosl = cos(lam);
        double e2   = gi.Geocent_e2;
        axi = sinp*sinp; axi = 1.0e0 - e2*axi; axi = gi.Geocent_a/sqrt(axi);
        dz=axi*e2*sinp;
    }
    else { // Cache provided
        h    = pCache->h;
        sinp = pCache->sinp;
        cosp = pCache->cosp;
        sinl = pCache->sinl;
        cosl = pCache->cosl;
        axi  = pCache->axi;
        dz   = pCache->dz;
    }

    // geodetic to geocentric
    XYZ xyzGc;
    Blh2Xyz(blhGeodetic,&xyzGc);

    // initialize topocentric coordinates
    pxyzTc->dX = xyzGc.dX;
    pxyzTc->dY = xyzGc.dY;

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
    pxyzTc->dZ = pxyzTc->dZ - (axi + h);

    // direct x to the North
    pxyzTc->dX = -pxyzTc->dX;

   // swap x and y
   double tmp=pxyzTc->dX; pxyzTc->dX=pxyzTc->dY; pxyzTc->dY=tmp;

   {
       Legacy_TdCord::SetEllipsParams();
       XYZ xyzGc;
       Legacy_TdCord::Blh2Xyz(blhGeodetic,&xyzGc);
       // qDebug() << "LegacyGeocentric: " <<  "(" << xyzGc.dX <<  ", " << xyzGc.dY <<  ", " << xyzGc.dZ <<  "); ";
   }
   return;
}
/*=========================================================
 * PBLH pblhViewPoint,BLH pblhGeodetic:
 * dLat,dLon in radians
 * dHei in meters
 * pxyzTc in meters
 *=========================================================*/
void TdCord::fromTopocentric(PBLH pblhViewPoint,XYZ xyzTc,PBLH pblhGeodetic,struct GeodeticToTopocentricCache *pCache /* = 0 */) {
    // transform coefficients -- may be cached
    double h,sinp,cosp,sinl,cosl,axi,dz;
    GeodeticToTopocentricCache *pCache_loc=0;
    if (!pCache) { // no cache provided
        // check view point pointer
        if (!pblhViewPoint) {
            throw RmoException("TdCord::toTopocentric - pblhViewPoint is NULL");
            return;
        }
        pCache_loc = new struct GeodeticToTopocentricCache;
        initGeodeticToTopocentricCache(pblhViewPoint,pCache_loc);
        pCache = pCache_loc;
    }
    h    = pCache->h;
    sinp = pCache->sinp;
    cosp = pCache->cosp;
    sinl = pCache->sinl;
    cosl = pCache->cosl;
    axi  = pCache->axi;  // gi.Geocent_a/sqrt(1.0e0 - e2*sinp*sinp)
    dz   = pCache->dz;

    // topocentric origin
    double axiPlusH,xTcO,yTcO,zTcO;
    axiPlusH = axi + h;
    xTcO = axiPlusH*cosp*cosl;
    yTcO = axiPlusH*cosp*sinl;
    double e2   = gi.Geocent_e2;
    zTcO = 1.0e0-e2; zTcO = zTcO*axi+h; zTcO = zTcO*sinp;

    // offset from topocentric origin due to xyzTc
    double xOff,yOff,zOff;
    xOff = -sinp*cosl*xyzTc.dY - sinl*xyzTc.dX + cosp*cosl*xyzTc.dZ;
    yOff = -sinp*sinl*xyzTc.dY + cosl*xyzTc.dX + cosp*sinl*xyzTc.dZ;
    zOff =  cosp*xyzTc.dY                      + sinp*xyzTc.dZ;

    // geocentric coordinates corresponding to topocentric (xyzTc)
    XYZ xyzGc;
    xyzGc.dX = xTcO + xOff;
    xyzGc.dY = yTcO + yOff;
    xyzGc.dZ = zTcO + zOff;

    // qDebug() << "Cache: " <<  "h=" << h;
    // qDebug() << "Geocentric: " <<  "(" << xyzGc.dX <<  ", " << xyzGc.dY <<  ", " << xyzGc.dZ <<  "); ";

    // transform to geodetic
    Xyz2Blh(xyzGc, pblhGeodetic);

    // free cache if previously allocated
    if (pCache_loc) delete pCache_loc;
}
