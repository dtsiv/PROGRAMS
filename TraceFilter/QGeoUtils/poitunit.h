#ifndef POITUNIT_H
#define POITUNIT_H
#include <QtCore/QCoreApplication>
#include <QList>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QString>
#include "qavtctrl.h"
#include "codograms.h"

#include "tdcord.h"

#define GEARTH          9.87  // ускорение свободного падения, м/(c*c);
#define VERYLARGEAMOUNT 1.e10    // Чтобы больше не было

typedef struct SOMEKOFFS_
{   double A, B, C, D, E, F, T1, T2, T4, T6;
} SOMEKOFFS, * PSOMEKOFFS;

//******************************************************************************
//
//******************************************************************************
class Legacy_TdCord
{
public:
    Legacy_TdCord() { }
    static void  SetEllipsParams(void);
    static void  Dirct2(double dLon, double dLat, double dAz,
                        double dDis, double * pLat, double * pLon, double * pDis);
    static void  Blh2Xyz(BLH blh, PXYZ pp);
    static void  GpnArc(double dLat1r, double dLat2r, double * pArc);
    static void  GpnLoa(double dDLr, double * pAz1r, double * pAz2r, double * pAo,
                        double * pBo, double * pSms);
    static void  GpnHri(double dLat1, double dLon1, double dLat2, double dLon2,
                        double * pAz1, double * pAz2, double * pS);
    static SOMEKOFFS sk;
    static double dMajor, dMinor, dFlat, dESq;    // semimajor axis equatorial
    static double dMagic, dRMaxinun, dRMinimum;
private:
};
//******************************************************************************
//
//******************************************************************************
typedef struct RXPOINT_
{   union {
    struct { PBLH p00, p01, p10, p11, p20, p21; };
    PBLH p[6];
    };
    double dTimeOffs, dAmp, D0, D1, D2, sD0, sD1, sD2;			// dTimeOffs in seconds
} RXPOINT, * PRXPOINT;
#define MAXPOSTNUM MAX_dwPosCount

class TPoiT : public Legacy_TdCord
{
public:
    TPoiT(unsigned __int64 uqTime, double dLat, double dLon);
    TPoiT(PPOIT ppoit);
    TPoiT(PPOITE ppoit);
    TPoiT(TPoiT * p);
    ~TPoiT()
    {   if(rx) delete[] rx;
    }
    bool  CalculateXY(void);
	bool  CalcEquation(PBLH p0, PBLH p1, PBLH p2, double D0, double D1, PXYPOINT ppt);
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
	unsigned __int64 m_uqTime;
	DWORD m_dwTu;
	unsigned long m_uFlags;
    BLH blh[MAXPOSTNUM + 1];
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

#endif // POITUNIT_H
