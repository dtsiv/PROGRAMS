#ifndef TPOIT_H
#define TPOIT_H
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#define _USE_MATH_DEFINES
#if !defined(_MATH_DEFINES_DEFINED)
#undef M_PI
#endif
#include <math.h>

#include <QtCore/QCoreApplication>
#include <QList>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QString>
#include <QDebug>

#include "codograms.h"

#ifndef DBGPPRINT
#define DBGPPRINT
inline void DbgPrint(const wchar_t * fmt, ...)
{   wchar_t buf[512];
    va_list list;

    va_start(list, fmt);
    vswprintf_s(buf, 512, fmt, list);
    va_end(list);
    OutputDebugString(buf);
}
#endif

#define GEARTH          9.87  // ускорение свободного падения, м/(c*c);
#define VERYLARGEAMOUNT 1.e10    // Чтобы больше не было
#define FREEPTREEDEPTH  5
#define RAD_TO_DEG	    57.29577951308232
#define DEG_TO_RAD	    0.0174532925199432958

#define ILL_OK          0
#define ILL_EPSILON     1
#define ILL_TOUT        2
#define ILL_VOVER       3
#define ILL_ZONE        4
#define ILL_USER        5
#define ILL_ADDR        6

typedef struct SOMEKOFFS_
{   double A, B, C, D, E, F, T1, T2, T4, T6;
} SOMEKOFFS, * PSOMEKOFFS;

#define __GNUG__
#include "cmatrix"
typedef techsoft::matrix<double>       Matrix;
typedef techsoft::matrix<double>       dMatrix;
typedef std::valarray<double>          dVector;
#define TRYBEGIN() try {
#define CATCHERROR() } catch (const std::exception& e) { \
						cerr << "Error: " << e.what() << endl; }
#define EPSYLONWINDOWLEN 4

class QPsVoi {
public:
	static int    m_UseTolerance;
	static double m_Tolerance;
};

//******************************************************************************
//
//******************************************************************************
class TdCord
{
public:
    TdCord() { }
    static void __fastcall SetEllipsParams(void);
    static void __fastcall Dirct2(double dLon, double dLat, double dAz,
                        double dDis, double * pLat, double * pLon, double * pDis);
    static void __fastcall Blh2Xyz(BLH blh, PXYZ pp);
    static void BlhToXyz(double dLat, double dLon, double dHei,
				  double * pdX, double * pdY, double * pdZ);
    static void BlhToXyz(PBLH pblh, PXYZ pxyz);
    static void __fastcall GpnArc(double dLat1r, double dLat2r, double * pArc);
    static void __fastcall GpnLoa(double dDLr, double * pAz1r, double * pAz2r, double * pAo,
                        double * pBo, double * pSms);
    static void __fastcall GpnHri(double dLat1, double dLon1, double dLat2, double dLon2,
                        double * pAz1, double * pAz2, double * pS);
    static void toTopocentric(BLH viewPoint,double *gc,double *tc);
    static SOMEKOFFS sk;
    static double dMajor, dMinor, dFlat, dESq;    // semimajor axis equatorial
    static double dOvrld, dBTKof, dTimeGate;
    static double dEpsylonLimit, dAmpLimit;
	static double dMagic, dDiscrPeriod, dRMaxinun, dRMinimum;
	static double dStrobMult2D, dStrobMult3D;
	static double dPreStrobMult2D, dPreStrobMult3D;
	static int iSectorsNum;
	static int iCashCount[6];
	static bool bStartAs3D;
	static bool bUseRxOffsets;
	static double dRxOffsets[5];
    static const double DEFMAJOR;
    static const double DEFMINOR;
private:
};

typedef struct RXPOINT_
{   union {
    struct { PBLH p00, p01, p10, p11, p20, p21; };
    PBLH p[6];
    };
    double dTimeOffs, dAmp, D0, D1, D2, sD0, sD1, sD2;			// dTimeOffs in seconds
} RXPOINT, * PRXPOINT;
#define MAXPOSTNUM 8

class TPoiT : public TdCord
{
public:
    TPoiT(unsigned __int64 uqTime, double dLat, double dLon);
    // TPoiT(PPOIT ppoit); // dtsiv: apparently this explicit constructor is not used
    TPoiT(PPOITE ppoit);
    TPoiT(TPoiT * p);
    ~TPoiT()
    {   if(rx) delete[] rx;
	    if(tc) delete[] tc;
		if(m_pRxInfoe) delete[] m_pRxInfoe;
    }
    bool __fastcall CalculateXY(void);
    bool            CalculateXY_baseSelection(void);
    bool isValidConfig(int iIdlePost,bool bP[]) {
		if (iIdlePost==1) return (bP[2]&&bP[3]&&bP[4]);
		if (iIdlePost==2) return (bP[1]&&bP[3]&&bP[4]);
		if (iIdlePost==3) return (bP[1]&&bP[2]&&bP[4]);
		if (iIdlePost==4) return (bP[1]&&bP[2]&&bP[3]);
		return false;
    }
	// solve 2D hyperbolic equation for selected active posts
	bool solve2D_active(int *iActive, double *dPaths);
    void initPostsTopocentricCoords();
    void setActivePosts(int iIdlePost, int iActive[]);
	bool __fastcall CalcEquation(PBLH p0, PBLH p1, PBLH p2, double D0, double D1, PXYPOINT ppt);
    PRXPOINT operator [] (int iIndex)
    {   if(iIndex >= Count)
        {   wprintf(L"Q-PsVoi: TPoiT index out of range index=%d cou=%d\n",
                             iIndex, Count);
            return(NULL);
        }
        return(&rx[iIndex]);
    }
	double minAmp(void)
	{	double dRet = 1.e10;
		int i = 0;
		while(i < Count)
		{	if(dRet > rx[i].dAmp) dRet = rx[i].dAmp;
			i++;
		}
		return(dRet);
	}
	bool IsHasBase(int iMinue, int iSubtra, int iIndx);
    QString calculatedPathDiffs(double dX, double dY);
    QString getBasesString();
	unsigned __int64 m_uqTime;
	DWORD m_dwTu;
	unsigned long m_uFlags;
    BLH blh[MAXPOSTNUM + 1];
	// topocentric coordinates of posts
	double *tc;
	// assumed target height
	double m_dHeight;
	// possible number of solutions: 0,1,2
	int m_nSol;
	double m_x[2],m_y[2];
	// copy of source match info
	RXINFOE *m_pRxInfoe;
	// number of matches
	int m_iMatchCount;

    BLH tag;
    int Count;
    bool bInvalid;
    bool bXYValid;
    bool m_bAmbigous;
    bool m_bAlterXY;
    int m_Sector;
    PRXPOINT rx;
	XYPOINT m_pt;

private:
	void PrintRxConfiguration(int iIndx);
	double offset(PPOITE ppoit, int i);
};              

#endif // TPOIT_H

