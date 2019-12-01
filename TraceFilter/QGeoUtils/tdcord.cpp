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
// BLH: radians, meters; XYZ: meters
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
void TdCord::Xyz2Blh(XYZ xyz, PBLH pp) {
    pj_Convert_Geocentric_To_Geodetic(
        &gi, xyz.dX, xyz.dY, xyz.dZ,
        &pp->dLat, &pp->dLon, &pp->dHei);
}
/*=========================================================
 * PBLH pblhViewPoint,PBLH pblhTg:
 * dLat,dLon in radians
 * dHei in meters
 *=========================================================*/
void TdCord::toTopocentric(PBLH pblhViewPoint,PBLH pblhTg,PXYZ pxyzTc) {
    double gc[3],tc[3];
    XYZ xyzGc;
    Blh2Xyz(*pblhTg,&xyzGc);
    gc[0]=xyzGc.dX; gc[1]=xyzGc.dY; gc[2]=xyzGc.dZ;
    toTopocentric(*pblhViewPoint,&gc[0],&tc[0]);
    pxyzTc->dX=tc[0]; pxyzTc->dY=tc[1]; pxyzTc->dZ=tc[2];
}
/*=========================================================
 * viewPoint: radians, meters
 * tc_loc[0],tc_loc[1],tc_loc[2] - topocentric x,y,z coordinates
 *                     (x to the North,
 *                      y to the East,
 *                      z up along the normal to ellipsoid
 * gc_loc[0],gc_loc[1],gc_loc[2] - geocentric x,y,z coordinates
 *                     (x to Equator crossing with Grinwich
 *                      y to Equator crossing with required eastern meridian
 *                      z to the North Pole
 * (See scan for more info)
 *=========================================================*/
void TdCord::toTopocentric(BLH viewPoint,
                   double *gc_loc,
                   double *tc_loc) {
    double phi=viewPoint.dLat;
    double lam=viewPoint.dLon;
    double h  =viewPoint.dHei;
    double sinp = sin(phi), cosp = cos(phi), sinl = sin(lam), cosl = cos(lam);
    double e2   = gi.Geocent_e2;
    double axi = sinp*sinp; axi = 1.0e0 - e2*axi; axi = gi.Geocent_a/sqrt(axi);


     // initialize topocentric coordinates
    tc_loc[0] = gc_loc[0];
    tc_loc[1] = gc_loc[1];
    double dz=axi*e2*sinp;

     // shift coordinate system down along Oz
    tc_loc[2] = gc_loc[2] + dz;

     // temporary storage of rotated coordinates
    double xr,yr,zr;

     // rotate around Oz by lambda counter-clockwise
    xr = cosl*tc_loc[0] + sinl*tc_loc[1];
    yr =-sinl*tc_loc[0] + cosl*tc_loc[1];
    tc_loc[0] = xr;
    tc_loc[1] = yr;

     // rotate around Oy by pi/2-phi counter-clockwise
    xr = sinp*tc_loc[0] - cosp*tc_loc[2];
    zr = cosp*tc_loc[0] + sinp*tc_loc[2];
    tc_loc[0] = xr;
    tc_loc[2] = zr;

     // shift along Oz by a*xi+h
    dz = axi + h;
    tc_loc[2] = tc_loc[2] - dz;

     // direct x to the North
    tc_loc[0] = -tc_loc[0];

    // swap x and y
    double tmp=tc_loc[0]; tc_loc[0]=tc_loc[1]; tc_loc[1]=tmp;
    return;
}

