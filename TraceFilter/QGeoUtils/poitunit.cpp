#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "poitunit.h"

#include <QtCore>

SOMEKOFFS Legacy_TdCord::sk;
double Legacy_TdCord::dMajor = 6378245.0, Legacy_TdCord::dMinor = 6356863.0;     // by Krasovsky default ellips
double Legacy_TdCord::dFlat, Legacy_TdCord::dESq;   // this will be set by static fn SetEllipsParams()
double Legacy_TdCord::dMagic = 1.;        // used for opaque manipulations with delays
double Legacy_TdCord::dRMaxinun = 500000.;// this is important for calculateXY() !!!
double Legacy_TdCord::dRMinimum = 5000.;  // this is important for calculateXY() !!!

//******************************************************************************
//
//******************************************************************************
void  Legacy_TdCord::SetEllipsParams() {
    // double d;

    dFlat = (dMajor - dMinor) / dMajor;
    dESq = dFlat * (2. - dFlat);

    double E4 = dESq * dESq;
    double E6 = E4 * dESq;
    double E8 = E6 * dESq;
    double EX = E8 * dESq;

    double T1 = dESq * (3.0 / 4.0);
    double T2 = E4 * (15.0 / 64.0);
    double T3 = E6 * (35.0 / 512.0);
    double T4 = E8 * (315.0 / 16384.0);
    double T5 = EX * (693.0 / 131072.0);

    sk.A  = 1.0 + T1 + 3.0 * T2 + 10.0 * T3 + 35.0 * T4 + 126.0 * T5;
    sk.B  = T1 + 4.0 * T2 + 15.0 * T3 + 56.0 * T4 + 210.0 * T5;
    sk.C  = T2 + 6.0 * T3 + 28.0 * T4 + 120.0 * T5;
    sk.D  = T3 + 8.0 * T4 + 45.0 * T5;
    sk.E  = T4 + 10.0 * T5;
    sk.F  = T5;

    double F2 = dFlat * dFlat;
    sk.T1 = 1.;
    sk.T2 = (-1. / 4.) * dFlat * (1. + dFlat + F2);
    sk.T4 = 3. / 16. * F2 * (1. + (9. / 4.) * dFlat);
    sk.T6 = (-25. / 128.) * F2 * dFlat;
}
//******************************************************************************
//
// *** SOLUTION OF THE GEODETIC DIRECT PROBLEM AFTER T.VINCENTY
// *** MODIFIED RAINSFORD'S METHOD WITH HELMERT'S ELLIPTICAL TERMS
// *** EFFECTIVE IN ANY AZIMUTH AND AT ANY DISTANCE SHORT OF ANTIPODAL
//
//******************************************************************************
void  Legacy_TdCord::Dirct2(double dLon, double dLat,
            double dAz, double dDis, double * pLat, double * pLon, double * pAz)
{
    double R = 1. - dFlat;
    double TU = R * sin(dLat) / cos(dLat);
    double SF = sin(dAz);
    double CF = cos(dAz);
    double BAZ = 0.;
    if(CF != 0.) BAZ = atan2(TU, CF) * 2.;
    double CU = 1. / sqrt(TU * TU + 1.);
    double SU = TU * CU;
    double SA = CU * SF;
    double C2A = -SA * SA + 1.;
    double X = sqrt((1. / R / R - 1.) * C2A + 1.) + 1.;
    X = (X - 2.) / X;
    double C = 1. - X;
    C = (X * X / 4. + 1.) / C;
    double D = (0.375 * X * X - 1.) * X;
    TU = dDis / R / dMajor / C;
    double Y = TU;
    double SY, CY, CZ, E;
    do
    {   SY = sin(Y);
        CY = cos(Y);
        CZ = cos(BAZ + Y);
        E = CZ * CZ * 2. - 1.;
        C = Y;
        X = E * CY;
        Y = E + E - 1.;
        Y = (((SY * SY * 4. - 3.) * Y * CZ * D / 6. + X) * D / 4. - CZ) * SY * D + TU;
    } while(fabs(Y - C) > 0.5E-13);
    BAZ = CU * CY * CF - SU * SY;
    C = R * _hypot(SA, BAZ);
    D = SU * CY + CU * SY * CF;
    *pLat = atan2(D , C);
    C = CU * CY - SU * SY * CF;
    X = atan2(SY * SF, C);
    C = ((-3. * C2A + 4.) * dFlat + 4.) * C2A * dFlat / 16.;
    D = ((E * CY * C + CZ) * SY * C + Y) * SA;
    *pLon = dLon + X - (1. - C) * D * dFlat;
    if(pAz) *pAz = (atan2(SA , BAZ) + M_PI);
}
//******************************************************************************
//
//******************************************************************************
void  Legacy_TdCord::Blh2Xyz(BLH blh, PXYZ pp)
{
	double cosB = cos(blh.dLat);   double sinB = sin(blh.dLat);
	double cosL = cos(blh.dLon);   double sinL = sin(blh.dLon);
	double dHei = blh.dHei;
	double ksi = 1. / sqrt(1. - dESq * sinB * sinB);
	pp -> dX = dMajor * ksi * cosB * cosL + dHei * cosB * cosL;
	pp -> dY = dMajor * ksi * cosB * sinL + dHei * cosB * sinL;
	pp -> dZ = dMajor * (1. - dESq) * ksi * sinB + dHei * sinB;
}
//******************************************************************************
// WRITTEN BY:  ROBERT (Sid) SAFFORD
// PURPOSE:     SUBROUTINE TO COMPUTE THE LENGTH OF A MERIDIONAL ARC
//              BETWEEN TWO LATITUDES
//******************************************************************************
void  Legacy_TdCord::GpnArc(double dLat1r, double dLat2r, double * pArc)
{

    double DA, DB, DC, DD, DE, DF;
    double S1 = fabs(dLat1r);
    double S2 = fabs(dLat2r);

    DA = (dLat2r - dLat1r);
    if(S2 < M_PI_2 + 5.e-15 && S2 >  M_PI_2 - 5.e-15 && S1 < 5.e-15) S2 = 0.;
    else
    {               //     COMPUTE THE LENGTH OF A MERIDIONAL ARC BETWEEN TWO LATITUDES
        DB = sin(dLat2r * 2.) - sin(dLat1r * 2.);
        DC = sin(dLat2r * 4.) - sin(dLat1r * 4.);
        DD = sin(dLat2r * 6.) - sin(dLat1r * 6.);
        DE = sin(dLat2r * 8.) - sin(dLat1r * 8.);
        DF = sin(dLat2r * 10.) - sin(dLat1r * 10.);
                    //     COMPUTE THE S2 PART OF THE SERIES EXPANSION
        S2 = -DB * sk.B / 2. + DC * sk.C / 4. - DD * sk.D / 6. +
                DE * sk.E / 8. - DF * sk.F / 10.;
    }
    S1 = DA * sk.A; //     COMPUTE THE S1 PART OF THE SERIES EXPANSION
    *pArc = dMajor * (1. - dESq) * (S1 + S2);    //     COMPUTE THE ARC LENGTH
}
//******************************************************************************
// WRITTEN BY:  ROBERT (Sid) SAFFORD
// PURPOSE:     SUBROUTINE TO COMPUTE THE LIFF-OFF-AZIMUTH CONSTANTS
//******************************************************************************
void  Legacy_TdCord::GpnLoa(double dDLr, double * pAz1r,
                       double * pAz2r, double * pAo, double * pBo, double * pSms)
{
    double CONS = (M_PI - fabs(dDLr)) / (M_PI * dFlat);
    double AZ = asin(CONS);             // COMPUTE AN APPROXIMATE AZ
    double S;

    int i = 0;
    while(i++ < 7)
    {   S = cos(AZ);
        double C2 = S * S;
        double C4 = C2 * C2;
        double C6 = C4 * C2;

        *pAo = sk.T1 + sk.T2 * C2 + sk.T4 * C4 + sk.T6 * C6;   // COMPUTE NEW AO
        S = asin(CONS / *pAo);
        if(fabs(S - AZ) < 5.e-15) break;
        AZ = S;
    }
    *pAz1r = S;
    if(dDLr < 0.) *pAz1r = 2. * M_PI - *pAz1r;
    *pAz2r = 2. * M_PI - *pAz1r;
                                   //     EQUATORIAL - GEODESIC  (S - s)   "SMS"

    double ESQP = dESq / (1. - dESq);
    S = cos(*pAz1r);

    double U2 = ESQP * S * S;
    double U4 = U2 * U2;
    double U6 = U4 * U2;
    double U8 = U6 * U2;

    double T2 = (1. / 4.) * U2;
    double T4 = (-3. / 64.) * U4;
    double T6 = (5. / 256.) * U6;
    double T8 = (-175. / 16384.) * U8;

    *pBo = sk.T1 + T2 + T4 + T6 + T8;
    S = sin(*pAz1r);
    *pSms = dMajor * M_PI * (1. - dFlat * fabs(S) * *pAo - *pBo * (1. - dFlat));
}
//******************************************************************************
// written by:  robert (sid) safford
// purpose:     subroutine to compute helmert rainsford inverse problem
//
//     solution of the geodetic inverse problem after t. vincenty
//     modified rainsford's method with helmert's elliptical terms
//     effective in any azimuth and at any distance short of antipocal
//     from/to stations must not be the geographic pole.
//
//******************************************************************************
void  Legacy_TdCord::GpnHri(double dLat1, double dLon1,
         double dLat2, double dLon2, double * pAz1, double * pAz2, double * pS)
{

//     test the longitude difference with approximately 0.000000001 arc seconds
    double dl = dLon2 - dLon1;
    if(fabs(dl) < 5.e-14)
    {   double dArc;
        GpnArc(dLat1, dLat2, &dArc);
        *pS = fabs(dArc);
        if(dLat2 > dLat1)
        {   *pAz1 = 0.;
            *pAz2 = M_PI;
        } else
        {   *pAz2 = 0.;
            *pAz1 = M_PI;
        }
        return;
    }
                                    // test for longitude over 180 degrees
    if(dl > 0.)
    {   if(dl >= M_PI && dl < 2. * M_PI) dl -= 2. * M_PI;
    } else if(dl < -M_PI && dl >= -2. * M_PI) dl += 2. * M_PI;
    double ss = fabs(dl);
    if(ss > M_PI) ss = 2. * M_PI - ss;

                        // compute the limit in longitude (alimit), it is equal
                        // to twice the distance from the equator to the pole,
                        // as measured along the equator (east/ewst)
    double alimit = M_PI * (1. - dFlat);
                        // test for anti-nodal difference
    if(ss >= alimit )
    {   double r1 = fabs(dLat1);
        double r2 = fabs(dLat2);
        if(!(r1 > 7.e-3 && r2 > 7.e-3))
        {   if(!(r1 < 5.e-14 && r2 > 7.e-3 || r2 < 5.e-14 && r1 > 7.e-3))
            {   if(r1 > 5.e-14 || r2 > 5.e-14)
                {   *pAz1 = 0.;
                    *pAz2 = 0.;
                    *pS = 0.;
                    return;
                }
                double az1, az2, aa, bb, sms;
                GpnLoa(dl, &az1, &az2, &aa, &bb, &sms);
                *pAz1 = az1;
                *pAz2 = az2;
                *pS = dMajor * fabs(dl) - sms;
                return;
            }
        }
    }

    double f0 = 1. - dFlat;
    double b = dMajor * f0;
    double epsq = dESq /(1. - dESq);
    double f4 = dFlat * dFlat * dFlat * dFlat;

    dl = dLon2 - dLon1;

    double u1 = f0 * sin(dLat1) / cos(dLat1);          //     the reduced latitudes
    double u2 = f0 * sin(dLat2) / cos(dLat2);

    u1 = atan(u1);
    u2 = atan(u2);

    double su1 = sin(u1);
    double cu1 = cos(u1);
    double su2 = sin(u2);
    double cu2 = cos(u2);

    double w, q2, q4, q6, r2, r3, sig, ssig, csig, slon, clon, sinalf;
    double s = 0., delta = 0.;
    int i = 0;
    while(i++ < 8)
    {   clon = cos(dl + s);
        slon = sin(dl + s);

        csig  = su1 * su2 + cu1 * cu2 * clon;
        ssig  = _hypot(slon * cu2, su2 * cu1 - su1 * cu2 * clon);
        sig = atan2(ssig, csig);
        sinalf = cu1 * cu2 * slon / ssig;
        w = (1. - sinalf * sinalf);
        double t4 = w * w;
        double t6 = w * t4;
        double ao = dFlat * (sk.T1 + sk.T2 * w + sk.T4 * t4 + sk.T6 * t6);
        double a2 = dFlat * (-w * sk.T2 - t4 * sk.T4 * 3. / 4. - t6 * sk.T6 * 3. / 2.);
        double a4 = dFlat * (sk.T4 * t4 / 6. +  15. * sk.T6 * t6 / 50.);
        double a6 = 5. * f4 * t6 / 768.;
        double qo = 0.;
        if(w > 5.e-15) qo = -2. * su1 * su2 / w;
        q2 = csig + qo;
        q4 = 2. * q2 * q2 - 1.;
        q6 = q2 * (4. * q2 * q2 - 3.);
        r2 = 2. * ssig * csig;
        r3 = ssig * (3. - 4. * ssig * ssig);
        s = sinalf * (ao * sig + a2 * ssig * q2 + a4 * r2 * q4 + a6 * r3 * q6);
        delta += s;
        if(fabs(delta) < 0.5e-13) break;
    }

    double z = epsq * w;
    double bo = 1. + z * (1. / 4. + z *(-3. / 64. + z * (5. / 256. - z * 175. / 16384.)));
    double b2 = z * (-1. / 4. + z * (1. / 16. + z * (-15. / 512. + z * 35. / 2048.)));
    double b4 = z * z * (-1. / 128. + z * (3. / 512. - z * 35. / 8192.));
    double b6 = z * z * z * (-1. / 1536. + z * 5. / 6144.);
    *pS = b * (bo * sig + b2 * ssig * q2 + b4 * r2 * q4 + b6 * r3 * q6);
    double az1, az2;

//     now compute the az1 & az2 for latitudes not on the equator

    if(!(fabs(su1) < 5.0e-15 && fabs(su2) < 5.0e-15))
    {   double tana1 =  slon * cu2 / (su2 * cu1 - clon * su1 * cu2);
        double sina1 =  sinalf / cu1;
        az1 = atan2(sina1, sina1 / tana1);

        double tana2 =  slon * cu1 / (su1 * cu2 - clon * su2 * cu1);
        double sina2 = -sinalf / cu2;
        az2 = M_PI - atan2(sina2, sina2 / tana2);
    } else
    {   if(dl > M_PI) dl -= 2. * M_PI;
        if(fabs(dl) > M_PI) dl += 2. * M_PI;
        az1 = M_PI_2;
        if(dl < 0.) az1 = 3. * az1;
        az2 = az1 + M_PI;
    }
    *pAz1 = az1;
    *pAz2 = az2;
    if(*pAz1 < -0.0) *pAz1 += 2. * M_PI;
    else if(*pAz1 >= 2. * M_PI) *pAz1 -= 2. * M_PI;
    if(*pAz2 < -0.0) *pAz2 += 2. * M_PI;
    else if(*pAz2 >= 2. * M_PI) *pAz2 -= 2. * M_PI;
}
//******************************************************************************
//
//******************************************************************************
TPoiT::TPoiT(unsigned __int64 uqTime, double dLat, double dLon)
{   m_uqTime = uqTime;
    tag.dLat = dLat;
    tag.dLon = dLon;
    tag.dHei = 0.;
    blh[0] = tag;
    blh[0].dLat -= M_PI / 1800.;
    blh[1] = tag;
    blh[1].dLat -= M_PI / 1800.;
    blh[2] = tag;
    blh[2].dLon += M_PI / 1800.;
    blh[3] = tag;
    blh[3].dLon -= M_PI / 1800.;
    blh[4] = tag;
    blh[4].dLat += M_PI / 1800.;
    blh[4].dLon -= M_PI / 1800.;
    bXYValid = false;
    bInvalid = false;
	m_bAmbigous = false;
	m_bAlterXY = false;
	m_Sector = 0;
	m_uFlags = POIT_FLAGS_BLH_VALID | POIT_FLAGS_RX_VALID | POIT_FLAGS_VIEWP_VALID;
    Count = 1;
    rx = new RXPOINT[Count];
    rx -> dTimeOffs = 0.;
    rx -> p00 = &blh[1];
    rx -> p01 = &blh[2];
    rx -> D0 = 0.;
    rx -> sD0 = 0.05;
    rx -> p10 = &blh[1];
    rx -> p11 = &blh[3];
    rx -> D1 = 0.;
    rx -> sD1 = 0.05;
    rx -> p20 = &blh[1];
    rx -> p21 = &blh[4];
    rx -> D2 = 0.;
    rx -> sD2 = 0.05;
}
//******************************************************************************
//
//******************************************************************************
TPoiT::TPoiT(PPOITE ppoit)
{   int i;

    m_uqTime = ppoit -> uqTlock;
    memcpy(&blh, &ppoit -> blh, sizeof(blh));
    bXYValid = false;
	m_bAlterXY = false;
	m_bAmbigous = false;
	bInvalid = true;
	tag.dHei = 5000.;
	m_Sector = ppoit -> iSector;
	m_uFlags = ppoit -> uFlags;
    rx = NULL;
    Count = 0;
	int iImpIndx = 0;
    if(ppoit -> Count == 0) {
        qDebug() << "Q-PsVoi: Error: NULL poit array";
        return;
    }
	i = 0;
	while(i < ppoit -> Count)
	{	if(ppoit -> rx[i++].iIndex == iImpIndx) continue;
		iImpIndx++;
	}
    if(ppoit -> rx[ppoit -> Count - 1].iIndex != iImpIndx) {
        qDebug() << "Q-PsVoi: Error: Invalid poit array structure RxCount=" << ppoit -> Count;
        return;
    }
	Count = iImpIndx + 1;
	rx = new RXPOINT[Count];
	PRXPOINT prx = &rx[0];
	i = 0;
	iImpIndx = 0;
	while(i < ppoit -> Count)
	{	prx -> dTimeOffs = 1.e-9 * (double)(long)ppoit -> rx[i].uT; // uT in nanoSec
        prx -> dAmp = ppoit -> rx[i].dAmp;
		prx -> p00 = &blh[ppoit -> rx[i].uMinuIndx];
        prx -> p01 = &blh[ppoit -> rx[i].uSubtIndx];
        prx -> D0 = ppoit -> rx[i].D + offset(ppoit, i);
        prx -> sD0 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
        prx -> p10 = NULL;
        prx -> p11 = NULL;
        prx -> D1 = 0.;
        prx -> sD1 = VERYLARGEAMOUNT;
        prx -> p20 = NULL;
        prx -> p21 = NULL;
        prx -> D2 = 0.;
        prx -> sD2 = VERYLARGEAMOUNT;
		if(++i >= ppoit -> Count) break;
        if(ppoit -> rx[i].iIndex == iImpIndx)
        {   prx -> p10 = &blh[ppoit -> rx[i].uMinuIndx];
            prx -> p11 = &blh[ppoit -> rx[i].uSubtIndx];
            prx -> D1 = ppoit -> rx[i].D + offset(ppoit, i);
            prx -> sD1 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
			if(ppoit -> rx[i].dAmp < prx -> dAmp) prx -> dAmp = ppoit -> rx[i].dAmp;
			if(++i >= ppoit -> Count) break;
        }
        if(ppoit -> rx[i].iIndex == iImpIndx)
        {   prx -> p20 = &blh[ppoit -> rx[i].uMinuIndx];
            prx -> p21 = &blh[ppoit -> rx[i].uSubtIndx];
            prx -> D2 = ppoit -> rx[i].D + offset(ppoit, i);
            prx -> sD2 = dMagic * max(ppoit -> rx[i].sD,  0.0001);
			if(ppoit -> rx[i].dAmp < prx -> dAmp) prx -> dAmp = ppoit -> rx[i].dAmp;
			if(++i >= ppoit -> Count) break;
        }
		while(ppoit -> rx[i].iIndex == iImpIndx) if(++i >= ppoit -> Count) break;
		prx++; 
		iImpIndx++;
	}
    bInvalid = false;
//	PrintRxConfiguration(0);
//	PrintRxConfiguration(1);
//	PrintRxConfiguration(2);
}
//******************************************************************************
//
//******************************************************************************
void TPoiT::PrintRxConfiguration(int iIndx)
{	if(iIndx >= Count) return;
	
	int ip[6];
	int j = 0;
	while(j < 6)
	{	ip[j] = 0;
		int i = 0;
		while(i < 7)
		{	if(rx[iIndx].p[j] == &blh[i])
			{	ip[j] = i;
				break;
			}
			i++;
		}
		j++;
	}
    qDebug() << QString("RX %1 offs=%2 congig D0 %3-%4, D1 %5-%6, D2 %7-%8")
                .arg(iIndx).arg(rx[iIndx].dTimeOffs).arg(ip[0]).arg(ip[1])
                .arg(ip[2]).arg(ip[3]).arg(ip[4]).arg(ip[5]);
}
//******************************************************************************
//
//******************************************************************************
bool TPoiT::IsHasBase(int iMinue, int iSubtra, int iIndx)
{	if(iIndx >= Count) return(false);
	PRXPOINT prx = &rx[iIndx];	

//	DbgPrint(L"++++++ minu %ld subtr %ld", iMinue, iSubtra);
	return(prx -> p00 == &blh[iMinue] && prx -> p01 == &blh[iSubtra] ||
		prx -> p10 == &blh[iMinue] && prx -> p11 == &blh[iSubtra] ||
		prx -> p20 == &blh[iMinue] && prx -> p21 == &blh[iSubtra]);
}
//******************************************************************************
//
//******************************************************************************
TPoiT::TPoiT(TPoiT * p)
{   *this = *p;
    rx = new RXPOINT[Count];
    memcpy(rx, &p -> rx[0], Count * sizeof(RXPOINT));
    int i = 0, j, k;
    while(i < Count)
    {   k = 0;
        while(k < 6)
        {   if(rx[i].p[k])
            {   j = 0;
                while(j++ < MAXPOSTNUM + 1) if(rx[i].p[k] == &p -> blh[j])
                {   rx[i].p[k] = &blh[j];
                    break;
                }
            }
            k++;
        }
        i++;
    }
}
//******************************************************************************
//
//******************************************************************************
bool  TPoiT::CalculateXY(void)
{   bXYValid = false;
	if(bInvalid) return(false);
    PRXPOINT prx = NULL;
	XYPOINT ptTag;
	PBLH p0, p1, p2;
	double D0, D1;
	int i;
	bool bCalcRet = false;
//	QString qs;
    
    if(!(m_uFlags & POIT_FLAGS_BLH_VALID && m_uFlags & POIT_FLAGS_RX_VALID && m_uFlags & POIT_FLAGS_VIEWP_VALID)) {
        qDebug() << "TPoiT::CalculateXY(void): m_uFlags are invalid!";
        return(false);
    }
	i = 0;
    while(i < Count)
    {   if(rx[i].p00 != NULL && rx[i].p01 != NULL && rx[i].p10 != NULL && rx[i].p11 != NULL)
        {	if(rx[i].p00 != rx[i].p01 && rx[i].p10 != rx[i].p11)
			{	prx = &rx[i];
				break;
			}
        }
        i++;
    }
    if(prx != NULL)
	{	if(prx -> p00 == prx -> p10)
		{	p0 = prx -> p00;
			p1 = prx -> p01;
			p2 = prx -> p11;
			D0 = prx -> D0;
			D1 = prx -> D1;
//			qs = QString("00 01 11");
		} else
		if(prx -> p00 == prx -> p11)
		{	p0 = prx -> p00;
			p1 = prx -> p01;
			p2 = prx -> p10;
			D0 = prx -> D0;
			D1 = -prx -> D1;
//			qs = QString("00 01 10");
		} else
		if(prx -> p01 == prx -> p10)
		{	p0 = prx -> p01;
			p1 = prx -> p00;
			p2 = prx -> p11;
			D0 = -prx -> D0;
			D1 = prx -> D1;
//			qs = QString("01 00 11");
		} else
		if(prx -> p01 == prx -> p11)
		{	p0 = prx -> p01;
			p1 = prx -> p00;
			p2 = prx -> p10;
			D0 = -prx -> D0;
			D1 = -prx -> D1;
//			qs = QString("01 00 10");
		}
		bCalcRet = CalcEquation(p0, p1, p2, D0, D1, &ptTag);
	}
	if(!bCalcRet)
	{	prx = NULL;
		i = 0;
		while(i < Count)
		{   if(rx[i].p00 != NULL && rx[i].p01 != NULL && rx[i].p20 != NULL && rx[i].p21 != NULL)
			{	if(rx[i].p00 != rx[i].p01 && rx[i].p20 != rx[i].p21)
				{	prx = &rx[i];
					break;
				}
			}
			i++;
		}
		if(prx != NULL)
		{	if(prx -> p00 == prx -> p20)
			{	p0 = prx -> p00;
				p1 = prx -> p01;
				p2 = prx -> p21;
				D0 = prx -> D0;
				D1 = prx -> D2;
//				qs = QString("00 01 21");
			} else
			if(prx -> p00 == prx -> p21)
			{	p0 = prx -> p00;
				p1 = prx -> p01;
				p2 = prx -> p20;
				D0 = prx -> D0;
				D1 = -prx -> D2;
//				qs = QString("00 01 20");
			} else
			if(prx -> p01 == prx -> p20)
			{	p0 = prx -> p01;
				p1 = prx -> p00;
				p2 = prx -> p21;
				D0 = -prx -> D0;
				D1 = prx -> D2;
//				qs = QString("01 00 21");
			} else
			if(prx -> p01 == prx -> p21)
			{	p0 = prx -> p01;
				p1 = prx -> p00;
				p2 = prx -> p20;
				D0 = -prx -> D0;
				D1 = -prx -> D2;
//				qs = QString("01 00 20");
			}
			bCalcRet = CalcEquation(p0, p1, p2, D0, D1, &ptTag);
		}
	}
	if(!bCalcRet)
	{	prx = NULL;
		i = 0;
		while(i < Count)
		{   if(rx[i].p10 != NULL && rx[i].p11 != NULL && rx[i].p20 != NULL && rx[i].p21 != NULL)
			{	if(rx[i].p10 != rx[i].p11 && rx[i].p20 != rx[i].p21)
				{	prx = &rx[i];
					break;
				}
			}
			i++;
		}
		if(prx != NULL)
		{	if(prx -> p10 == prx -> p20)
			{	p0 = prx -> p10;
				p1 = prx -> p11;
				p2 = prx -> p21;
				D0 = prx -> D1;
				D1 = prx -> D2;
//				qs = QString("10 11 21");
			} else
			if(prx -> p10 == prx -> p21)
			{	p0 = prx -> p10;
				p1 = prx -> p11;
				p2 = prx -> p20;
				D0 = prx -> D1;
				D1 = -prx -> D2;
//				qs = QString("10 11 20");
			} else
			if(prx -> p11 == prx -> p20)
			{	p0 = prx -> p11;
				p1 = prx -> p10;
				p2 = prx -> p21;
				D0 = -prx -> D1;
				D1 = prx -> D2;
//				qs = QString("11 10 21");
			} else
			if(prx -> p11 == prx -> p21)
			{	p0 = prx -> p11;
				p1 = prx -> p10;
				p2 = prx -> p20;
				D0 = -prx -> D1;
				D1 = -prx -> D2;
//				qs = QString("11 10 20");
			}
			bCalcRet = CalcEquation(p0, p1, p2, D0, D1, &ptTag);
//			DbgPrint(L"Q-PsVoi: III calc=%ld", bCalcRet);
		}
	}
	if(!bCalcRet)
	{	/*
		DbgPrint(L"Q-PsVoi: Calculate XY ERROR: Invalid positions configuration, count=%ld", Count);
		PrintRxConfiguration(0);
		PrintRxConfiguration(1);
		PrintRxConfiguration(2);***/
        // qDebug() << "TPoiT::CalculateXY(void): !bCalcRet";
		return(false);
    } //else DbgPrint(L"Q-PsVoi: calc=ok %ls", qs.utf16());

    double dS = _hypot(ptTag.dX, ptTag.dY);
    if(dS > dRMaxinun || dS < dRMinimum) {	//DbgPrint(L"Q-PsVoi: Calculate XY Out of zone R=%0.2lf km", dS * 0.001);
        // qDebug() << "TPoiT::CalculateXY(void): dS > dRMaxinun || dS < dRMinimum -- "
        //          << QString("%1 < %2 < %3").arg(dRMinimum).arg(dS).arg(dRMaxinun);
        return(false);
	}    
	double dA = atan2(-ptTag.dX, -ptTag.dY) + M_PI;
    if(dA > 2. * M_PI) dA -= 2. * M_PI;
    else if(dA < 0.) dA += 2. * M_PI;
    Dirct2(blh[0].dLon, blh[0].dLat, dA, dS, &tag.dLat, &tag.dLon, NULL);
	m_pt = ptTag;
	m_dwTu = (int)(prx -> dTimeOffs * 1.e9) / 100; 
	bXYValid = true;
	return(true);
}
//******************************************************************************
//
//******************************************************************************
bool  TPoiT::CalcEquation(PBLH p0, PBLH p1, PBLH p2, double D0, double D1, PXYPOINT ppt)
{   
    double dS, dA, dAback;
    GpnHri(blh[0].dLat, blh[0].dLon, p0 -> dLat, p0 -> dLon, &dA, &dAback, &dS);
    double x0 = dS * sin(dA), y0 = dS * cos(dA);
    GpnHri(p0 -> dLat, p0 -> dLon, p1 -> dLat, p1 -> dLon, &dA, &dAback, &dS);
    double xl = dS * sin(dA), yl = dS * cos(dA);
    GpnHri(p0 -> dLat, p0 -> dLon, p2 -> dLat, p2 -> dLon, &dA, &dAback, &dS);
    double xr = dS * sin(dA), yr = dS * cos(dA);

    double a1, a2, m = xl * yr  - xr * yl;
    a1  = yr * D0 - yl * D1;
    a2  = xl * D1 - xr * D0;

    a1 /= m;
	a2 /= m;
	m *= 2.;

    double dd  =  xl * xl + yl * yl - D0 * D0;
    double dd_ =  xr * xr + yr * yr - D1 * D1;

    double b1  =  yr * dd - yl * dd_;
           b1 /=  m;
    double b2  =  xl * dd_ - xr * dd;
           b2 /=  m;

    dd_ = 1. - a1 * a1 - a2 * a2;
    dd  = a1 * b1 + a2 * b2;
									// Расчёт дискриминанта квадратного уравнения
									// относительно расстояния между Целью и
									// Центральной ("нулевой") приёмной
									// позицией (дальности до цели):
    double _dd = dd * dd + dd_ * (b1 * b1 + b2 * b2);
    if(_dd < 0.) return(false);
    _dd = sqrt(_dd);				// дискриминант квадратного уравнения

    double rPlus  = (dd + _dd) / dd_;
    double rMinus = (dd - _dd) / dd_;
	if(rPlus >= 0. && rMinus >= 0.) return(false);
	if(rPlus >= 0. && rMinus < 0.)
    {   ppt -> dX = x0 + a1 * rPlus + b1;
        ppt -> dY = y0 + a2 * rPlus + b2;
//		DbgPrint(L"Q-PsVoi: rPlus +++");
		return(true);
    }
	if(rMinus >= 0. && rPlus < 0.)
    {   ppt -> dX = x0 + a1 * rMinus + b1;
        ppt -> dY = y0 + a2 * rMinus + b2;
//		DbgPrint(L"Q-PsVoi: rMinus ---");
        return(true);
    }
    return(false);
}
//******************************************************************************
//
//******************************************************************************
double TPoiT::offset([[maybe_unused]]PPOITE ppoit, [[maybe_unused]]int indx) {
    return(0.);
}

