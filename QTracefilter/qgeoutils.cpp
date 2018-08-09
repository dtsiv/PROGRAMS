#include "qgeoutils.h"

#include <QtDebug>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::QGeoUtils(QObject *parent /*=0*/)
	: QObject(parent) 
	, NUMBER_OF_POSTS(4)
    , NPAIRS(5)
    , RAD_2_DEG(57.29577951308232)
    , DEG_2_RAD(.0174532925199432958)
    , DEFMAJOR(6378245.0)
    , DEFMINOR(6356863.0)    // by Krasovsky default ellips
    , EllipsMajor(6378245.00)
    , EllipsMinor(6356863.00)
    // light speed in m/ns
    , SPEED_OF_LIGHT(0.299792458e0)
    , MISMATCH_THRESH(1.0e6)
    , EPS(1.0e-8)
  {
	tc = new double[4*NUMBER_OF_POSTS];
	gc = new double[4*NUMBER_OF_POSTS];
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::QGeoUtils(const QGeoUtils &guTemplate, QObject *parent /*=0*/)
	: QObject(parent) 
	, NUMBER_OF_POSTS(guTemplate.NUMBER_OF_POSTS)
    , NPAIRS(guTemplate.NPAIRS)
    , RAD_2_DEG(guTemplate.RAD_2_DEG)
    , DEG_2_RAD(guTemplate.DEG_2_RAD)
    , DEFMAJOR(guTemplate.DEFMAJOR)
    , DEFMINOR(guTemplate.DEFMINOR)    // by Krasovsky default ellips
    , EllipsMajor(guTemplate.EllipsMajor)
    , EllipsMinor(guTemplate.EllipsMinor)
    // light speed in m/ns
    , SPEED_OF_LIGHT(guTemplate.SPEED_OF_LIGHT)
    , MISMATCH_THRESH(guTemplate.MISMATCH_THRESH)
    , EPS(guTemplate.EPS)
	, m_pMainCtrl(NULL)
  {
	m_fMainctrlCfg.setFileName(guTemplate.m_fMainctrlCfg.fileName());
	tc = new double[4*NUMBER_OF_POSTS];
	memcpy(tc,guTemplate.tc,4*NUMBER_OF_POSTS*sizeof(double));
	gc = new double[4*NUMBER_OF_POSTS];
	memcpy(gc,guTemplate.gc,4*NUMBER_OF_POSTS*sizeof(double));
	if (m_fMainctrlCfg.isOpen()) {
		m_pMainCtrl=(PMAINCTRL)m_fMainctrlCfg.map(0,m_fMainctrlCfg.size());
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::~QGeoUtils() {
	delete[] tc;
	delete[] gc;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   pblhViewPoint - radians, meters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::getViewPoint(PBLH pblhViewPoint, PMAINCTRL pMainCtrl) {
	pblhViewPoint->dLat=0.0e0; pblhViewPoint->dLon=0.0e0; pblhViewPoint->dHei=0.0e0; 
	if (pMainCtrl->p.dwPosCount < 5) return;
	for (int i=1; i<=4; i++) {
		PGROUNDINFO pgi = &pMainCtrl -> p.positions[i];
		pblhViewPoint->dLat += pgi->blh.dLat*DEG_2_RAD;
		pblhViewPoint->dLon += pgi->blh.dLon*DEG_2_RAD;
	}
	pblhViewPoint->dLat/=4.0e0; pblhViewPoint->dLon/=4.0e0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QGeoUtils::readCfgFile(QString qsMainctrlCfg) {
//==================== post config =====================
	if (qsMainctrlCfg.isEmpty()) return false;
	m_fMainctrlCfg.setFileName(qsMainctrlCfg);
	if (!m_fMainctrlCfg.open(QIODevice::ReadOnly)) { qDebug() << "Open qsMainctrlCfg failed: " << qsMainctrlCfg; return false; };
    m_pMainCtrl=(PMAINCTRL)m_fMainctrlCfg.map(0,m_fMainctrlCfg.size());
	int ii;
	for (ii=1; ii<=4; ii++) {
	    XYZ postXYZ;
		PGROUNDINFO pgi = &m_pMainCtrl -> p.positions[ii];
		m_blhPosts[ii]=pgi->blh;
        BlhToXyz(&pgi->blh,&postXYZ);
		gc[3*ii+0]=postXYZ.dX;
		gc[3*ii+1]=postXYZ.dY;
		gc[3*ii+2]=postXYZ.dZ;
	}
    BLH post4;
    post4.dLat=m_pMainCtrl->p.positions[4].blh.dLat;
    post4.dLon=m_pMainCtrl->p.positions[4].blh.dLon;
    post4.dHei=m_pMainCtrl->p.positions[4].blh.dHei;
	m_fMainctrlCfg.close();

	for (ii=1; ii<=4; ii++) {
        toTopocentric(post4,gc+3*ii,tc+3*ii);
	}

	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QGeoUtils::isValidConfig(int iIdlePost,bool bP[]) {
	if (iIdlePost==0) return (bP[1]&&bP[2]&&bP[3]);
	if (iIdlePost==1) return (bP[0]&&bP[2]&&bP[3]);
	if (iIdlePost==2) return (bP[0]&&bP[1]&&bP[3]);
	if (iIdlePost==3) return (bP[0]&&bP[1]&&bP[2]);
	return false;
}
//******************************************************************************
//                Прямой расчёт  геоцентрических координат X, Y, Z    [м]
//                по известным   геодезическим координатам B, L, h
//					B, L [градусы], h [м]				
// m_dMajor м, большая полуось Земли
// m_dESq квадрат эксцентриситета Земли
//******************************************************************************
void QGeoUtils::BlhToXyz(double dLat, double dLon, double dHei,
									double * pdX, double * pdY, double * pdZ)
{   
	double dMajor = DEFMAJOR;
	double dFlat = (dMajor - DEFMINOR) / dMajor;
	double m_dMajor = dMajor;
    double m_dESq = dFlat * (2. - dFlat);
	
	dLon *= DEG_2_RAD;
    dLat *= DEG_2_RAD;
	double cosB = cos(dLat);   double sinB = sin(dLat);
	double cosL = cos(dLon);   double sinL = sin(dLon);
	double ksi = 1. / sqrt(1. - m_dESq * sinB * sinB);
	if(pdX) *pdX = m_dMajor * ksi * cosB * cosL + dHei * cosB * cosL;
	if(pdY) *pdY = m_dMajor * ksi * cosB * sinL + dHei * cosB * sinL;
	if(pdZ) *pdZ = m_dMajor * (1. - m_dESq) * ksi * sinB + dHei * sinB;
}
void QGeoUtils::BlhToXyz(PBLH pblh, PXYZ pxyz)
{
	BlhToXyz(pblh -> dLat, pblh -> dLon, pblh -> dHei, &pxyz -> dX, &pxyz -> dY, &pxyz -> dZ);
}

//******************************************************************************
//                Обратный расчёт геоцентрических координат   B, L,[градусы]; h, [м];
//                по известным    геоцентрическим координатам X, Y, Z, [м].
//                
//******************************************************************************
void QGeoUtils::XyzToBlh(double dX, double dY, double dZ,
							double * pdLat, double * pdLon, double * pdHei)
{
	double dMajor = DEFMAJOR;
	double dFlat = (dMajor - DEFMINOR) / dMajor;
	double m_dMajor = dMajor;
    double m_dESq = dFlat * (2. - dFlat);

	double  m  = sqrt(dX * dX + dY * dY);
	double fi;
	if(m) fi = atan(dZ / m);   // Надо позаботиться о нуле (исключительная ситуация), например, с помощью  функции atan2
	else fi = 0.;
	if(pdLon) *pdLon = RAD_2_DEG * atan2(dY, dX);

	double  ksi;
	double Q;                 // , м - Радиус кривизны меридианального сечения

	double deltaBMeter = 1e10;       //  Текущая ошибка оценки B, выраженная в метрах.
	double deltaHMeter = 1e10;       //  Текущая ошибка оценки Д, выраженная в метрах.

			// 0.01 Максимально допустимая ошибка оценки B, выраженная в метрах.
			// 0.01 Максимально допустимая ошибка оценки h, выраженная в метрах.

 	int i = 0;                   // Счётчик числа итераций
	double B, B_ = fi;
	double H, H_ = 0.;

	while((abs(deltaBMeter) > 0.01 || abs(deltaHMeter) > 0.01) && i < 13)
	{	
		double  cosB = cos(B_);
		double  sinB = sin(B_);
		ksi = 1. / sqrt(1. - m_dESq * sinB * sinB);
		double K = dZ - (1. - m_dESq) * m_dMajor * ksi * sinB; 
		H = sqrt((m - m_dMajor * ksi * cosB) * (m - m_dMajor * ksi * cosB) + K * K);

        B = atan(dZ * (m_dMajor * ksi + H_) / (m * (m_dMajor * (1. - m_dESq) * ksi + H_)));
		Q = m_dMajor * (1. - m_dESq) * ksi * ksi * ksi + H_;
		
		deltaBMeter = Q  * (B - B_);
		deltaHMeter = H - H_;
		B_ = B;
		H_ = H;

	}
	if(pdLat) *pdLat = RAD_2_DEG * B;
	if(pdHei) *pdHei = H;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::XyzToBlh(PXYZ pxyz, PBLH pblh)
{
	XyzToBlh(pxyz -> dX, pxyz -> dY, pxyz -> dZ, &pblh -> dLat, &pblh -> dLon, &pblh -> dHei);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double QGeoUtils::determinant3x3(double mat[9]) {
    /*   a11     a12      a13
     *  
     *   a21     a22      a23
     *
     *   a31     a32      a33   */
    int i,j;
    i=1; j=1; double a11=mat[3*(i-1)+j-1];
    i=1; j=2; double a12=mat[3*(i-1)+j-1];
    i=1; j=3; double a13=mat[3*(i-1)+j-1];
    i=2; j=1; double a21=mat[3*(i-1)+j-1];
    i=2; j=2; double a22=mat[3*(i-1)+j-1];
    i=2; j=3; double a23=mat[3*(i-1)+j-1];
    i=3; j=1; double a31=mat[3*(i-1)+j-1];
    i=3; j=2; double a32=mat[3*(i-1)+j-1];
    i=3; j=3; double a33=mat[3*(i-1)+j-1];

    double t1 = a11*a22*a33;
    double t2 = a11*a23*a32;
    double t3 = a21*a12*a33;
    double t4 = a21*a32*a13;
    double t5 = a31*a12*a23;
    double t6 = a31*a22*a13;

    return t1-t2-t3+t4+t5-t6;
}
/*=========================================================
 * PBLH pblhViewPoint,PBLH pblhTg: 
 * dLat,dLon in radians
 * dHei in meters
 *=========================================================*/
void QGeoUtils::toTopocentric(PBLH pblhViewPoint,PBLH pblhTg,PXYZ pxyzTc) {
	double gc[3],tc[3];
	XYZ xyzGc;
	BLH blhTgDeg;
	blhTgDeg.dLat=pblhTg->dLat*RAD_2_DEG;
	blhTgDeg.dLon=pblhTg->dLon*RAD_2_DEG;
	blhTgDeg.dHei=pblhTg->dHei;
	BlhToXyz(&blhTgDeg,&xyzGc);
	gc[0]=xyzGc.dX; gc[1]=xyzGc.dY; gc[2]=xyzGc.dZ;
	BLH blhViewPointDeg;
	blhViewPointDeg.dLat=pblhViewPoint->dLat*RAD_2_DEG;
	blhViewPointDeg.dLon=pblhViewPoint->dLon*RAD_2_DEG;
	blhViewPointDeg.dHei=pblhViewPoint->dHei;
	/*
	qDebug() << "ViewPoint: "
		<< blhViewPointDeg.dLat
		<< " " << blhViewPointDeg.dLon
		<< " " << blhViewPointDeg.dHei;
	*/
	toTopocentric(blhViewPointDeg,&gc[0],&tc[0]);
	pxyzTc->dX=tc[0]; pxyzTc->dY=tc[1]; pxyzTc->dZ=tc[2];
}
/*=========================================================
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
void QGeoUtils::toTopocentric(BLH viewPoint,
                   double *gc_loc,
                   double *tc_loc) {
// excentrisitet
    double eps=DEFMINOR/DEFMAJOR;
    eps=1.0e0-eps*eps;
    eps=sqrt(eps);
    double phi=DEG_2_RAD*viewPoint.dLat;
    double lam=DEG_2_RAD*viewPoint.dLon;
    double h  =viewPoint.dHei;
    double sinp = sin(phi), cosp = cos(phi), sinl = sin(lam), cosl = cos(lam);
    double e2   = eps*eps;
    double axi = sinp*sinp; axi = 1.0e0 - e2*axi; axi = DEFMAJOR/sqrt(axi);


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
