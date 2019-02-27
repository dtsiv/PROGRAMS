#!/usr/bin/python
# This Python file uses the following encoding: utf-8
import re

#========================================================================
#
#   define the required replacements in codograms.h
#
#========================================================================
codograms_changes = [] 

#======================= @line 10 =======================
pattern = """\
#define CODOGRAMS_H_VER_MAJOR 3
#define CODOGRAMS_H_VER_MINOR 5

"""

replacement = """\
#define CODOGRAMS_H_VER_MAJOR 3
#define CODOGRAMS_H_VER_MINOR 5

#ifndef __GNUC__
#error "Not a GNU C"
#endif

#ifndef __linux
#error "Not a linux"
#endif

#ifndef RCTRLH
#if __GNUC__
#ifdef __linux
    #define BYTE unsigned char
    #define USHORT unsigned short
    #define DWORD unsigned int
    #define PDWORD unsigned int*
    #define __int64 long long
#endif
#endif
#endif

"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 117 =======================
pattern = """\
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
    wchar_t sName[64];
    int iId;
    unsigned long uFlags;			// uFlags & 0xff - type, uFlags & ~0xff - GRINF_FLAG_XXX 
} GROUNDINFO, * PGROUNDINFO;
"""

replacement = """\
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
    unsigned short sName[64];
    int iId;
    unsigned int uFlags;			// uFlags & 0xff - type, uFlags & ~0xff - GRINF_FLAG_XXX 
} GROUNDINFO, * PGROUNDINFO;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 162 =======================
pattern = """\
typedef struct LOCALCTRLSTRUCT_
{	DWORD dwFlags;
	int iSelected;
	int iCount;
	DWORD dwRegisters[8];
	struct {
		wchar_t sModeName[48];
		int iInternalIndx;
	} mode[32];
} LOCALCTRLSTRUCT, * PLOCALCTRLSTRUCT;
"""

replacement = """\
typedef struct LOCALCTRLSTRUCT_
{	DWORD dwFlags;
	int iSelected;
	int iCount;
	DWORD dwRegisters[8];
	struct {
		unsigned short sModeName[48];
		int iInternalIndx;
	} mode[32];
} LOCALCTRLSTRUCT, * PLOCALCTRLSTRUCT;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 175 =======================
pattern = """\
typedef struct LOCALCTRLSTRUCTEX_
{	DWORD dwFlags;
	int iSelected;
	int iCount;
	DWORD dwRegisters[8];
	struct {
		wchar_t sModeName[48];
		char sConventions[256];
		int iInternalIndx;
	} mode[32];
} LOCALCTRLSTRUCTEX, * PLOCALCTRLSTRUCTEX;
"""

replacement = """\
typedef struct LOCALCTRLSTRUCTEX_
{	DWORD dwFlags;
	int iSelected;
	int iCount;
	DWORD dwRegisters[8];
	struct {
		unsigned short sModeName[48];
		char sConventions[256];
		int iInternalIndx;
	} mode[32];
} LOCALCTRLSTRUCTEX, * PLOCALCTRLSTRUCTEX;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 220 =======================
pattern = """\
typedef struct CONSOLEMSG_
{   union {
	FILETIME ftTime;				// Время (100ns ticks)
    unsigned __int64 uqTime;
	};
	int iId;
	wchar_t sMessage[];
} CONSOLEMSG, * PCONSOLEMSG;
"""

replacement = """\
typedef struct CONSOLEMSG_
{
    unsigned __int64 uqTime; 			// Время (100ns ticks)
	int iId;
	unsigned short sMessage[];
} CONSOLEMSG, * PCONSOLEMSG;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 232 =======================
pattern = """\
typedef struct NEWCONNECTION_		// wtype WTYP_NEWCONNECTION
{   union {
	FILETIME ftTime;				// Время UTC (100ns ticks)
    unsigned __int64 uqTime;
	};
	int iSocket;
	DWORD dwFlags;
} NEWCONNECTION, * PNEWCONNECTION;
"""

replacement = """\
typedef struct NEWCONNECTION_		// wtype WTYP_NEWCONNECTION
{
    unsigned __int64 uqTime;        // Время UTC (100ns ticks)
	int iSocket;
	DWORD dwFlags;
} NEWCONNECTION, * PNEWCONNECTION;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 264 =======================
pattern = """\
typedef struct RMOCOMMAND_  // dwType=4000
{   unsigned long Id;       // код команды VVOD1, SBROSOPR, KORR
	unsigned long rm;       // номер рабочего места
	unsigned long uNum;     // Trace Num
	unsigned long uFlags;   // Err code
	union {
		struct {
			double dLat;            // Lat маркера гр. или высота км
			double dLon;     	    // Lon маркера
		};
		struct {
			double dTStart;         // Начальное время сек, 0. - начало 
			double dTEnd;     	    // Конечное время сек, -1. - до конца
		};
	};
} RMOCOMMAND, * PRMOCOMMAND;
"""

replacement = """\
typedef struct RMOCOMMAND_  // dwType=4000
{   unsigned int Id;       // код команды VVOD1, SBROSOPR, KORR
	unsigned int rm;       // номер рабочего места
	unsigned int uNum;     // Trace Num
	unsigned int uFlags;   // Err code
	union {
		struct {
			double dLat;            // Lat маркера гр. или высота км
			double dLon;     	    // Lon маркера
		};
		struct {
			double dTStart;         // Начальное время сек, 0. - начало 
			double dTEnd;     	    // Конечное время сек, -1. - до конца
		};
	};
} RMOCOMMAND, * PRMOCOMMAND;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 303 =======================
pattern = """\
typedef struct FRSELECTION_		// dwType=WTYP_FRSELECTION (4002)
{	unsigned long rm;			// номер рабочего места
	unsigned long id;			// номер линейки частотной панорамы 
	double dFSel;				// центральная частота МГц
	double dBandSel;			// полуширина МГц	
	double dAzFrom;				// азимут от, гр. 
	double dAzTo;				// азимут до, гр.
	int iCPCount;				// число точек в осциллографической части
	int iFlags;					//флаги FRSEL_FLAG_XXX
	int iReserved[4];
} FRSELECTION, * PFRSELECTION;
"""

replacement = """\
typedef struct FRSELECTION_		// dwType=WTYP_FRSELECTION (4002)
{	unsigned int rm;			// номер рабочего места
	unsigned int id;			// номер линейки частотной панорамы 
	double dFSel;				// центральная частота МГц
	double dBandSel;			// полуширина МГц	
	double dAzFrom;				// азимут от, гр. 
	double dAzTo;				// азимут до, гр.
	int iCPCount;				// число точек в осциллографической части
	int iFlags;					//флаги FRSEL_FLAG_XXX
	int iReserved[4];
} FRSELECTION, * PFRSELECTION;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 321 =======================
pattern = """\
typedef struct ETVOI_               // dwType = WTYP_ET (1000)
{   union {
	FILETIME ftTime;				// Время экстраполяции (100ns ticks)
    unsigned __int64 uqTime;
	};
	unsigned long uTrNum;	        // Номер трассы
    unsigned short usFlags;         // флаги ET_FLAGS_ХХХХ
    unsigned short usStyle;         // дополнительный параметр (стиль отображения)
	union {
	double dAzimuth;	
	LBH lbh;						// радианы и км	
	};
    XYH velo;                       // скорости в км/c
	int iSmodeAdr;					// S-mode bort adress
	int iReserved;
	char bSmodeCall[8];				// S-mode call sign
	DWORD dwReserved[8];
} ETVOI, * PETVOI;
"""

replacement = """\
typedef struct ETVOI_               // dwType = WTYP_ET (1000)
{
    unsigned __int64 uqTime;        // Время экстраполяции (100ns ticks)
	unsigned int uTrNum;	        // Номер трассы
    unsigned short usFlags;         // флаги ET_FLAGS_ХХХХ
    unsigned short usStyle;         // дополнительный параметр (стиль отображения)
	union {
	double dAzimuth;	
	LBH lbh;						// радианы и км	
	};
    XYH velo;                       // скорости в км/c
	int iSmodeAdr;					// S-mode bort adress
	int iReserved;
	char bSmodeCall[8];				// S-mode call sign
	DWORD dwReserved[8];
} ETVOI, * PETVOI;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 357 =======================
pattern = """\
typedef union POIT_EXINFO_
{   struct {                    // RL,TACAN (данные пассивной разностно дальномерной системы)
	    unsigned Dummy0    :3;  // пусто
	    unsigned Mod       :3;  // код модуляции (1-ФКМ,2-ЧМ-,4-ЧМ+)
	    unsigned Pol       :2;  // код поляризации
	    unsigned Pim       :1;  // признак имитации
	    unsigned PmodT     :1;  // признак модуляции Т
	    unsigned PmodTau   :1;  // признак модуляции Тау
        unsigned B		   :1;  /*Вид сигнала (при tau > 400 мкс B = 1, иначе B = 0)*/
        unsigned H		   :1;  /*Признак наложения импульсов*/
        unsigned ModF      :1;  /*Признак модуляции по девиации (введен мною)*/
	    unsigned NTacan    :18; //
		union {
		int Sector;
		int PostID;
		};
		float Aimp;             // Амплитуда импульса Дб
	    float sigmataui;        // СКО длительности импульса мкс
	    float sigmaT;           // СКО периода повторения импульсов мкс
	    float sigmafc;          // СКО несущей частоты
    };

    struct {                    // SIF (данные пассивной разностно дальномерной системы)
	    unsigned long uDummyS;		// пусто
	    int Sector;
	    float Aimp;             // Амплитуда импульса Дб
        unsigned D4_C1	   :12; /*Позиционный код посылки*/
        unsigned UBD	   :17; /*Позиционный код режима УВД-М*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (3)*/
   /*Слово 1*/
        unsigned DP1	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (4)*/
   /*Слово 2*/
        unsigned DP2	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (5)*/
   /*Слово 3*/
        unsigned DP3	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (6)*/
   /*Слово 4*/
	    unsigned long Rezhim;   // режима SIF
    };

    struct {						// данные активной системы
	    unsigned short uDummyA;		// пусто
	    unsigned short uFlags;		// пусто
	    float fR, fBeta, fEps, fVdop;       // дальность, азимут, угол места, радиальная скорость
	    float fDR, fDBeta, fDEps, fDVdop;   // соответствующие сигмы ошибок этих велечин
        float fAmp;
        BYTE bBeem;
        BYTE bSId;
        BYTE bPId;
        BYTE bCId;
        DWORD dwDopMask;
	    float DummyA[5];        // период повторения импульсов мкс (для имитатора)
    };
} POIT_EXINFO, * PPOIT_EXINFO;
"""

replacement = """\
typedef union POIT_EXINFO_
{   struct anonymous_struct1 {                    // RL,TACAN (данные пассивной разностно дальномерной системы)
	    unsigned Dummy0    :3;  // пусто
	    unsigned Mod       :3;  // код модуляции (1-ФКМ,2-ЧМ-,4-ЧМ+)
	    unsigned Pol       :2;  // код поляризации
	    unsigned Pim       :1;  // признак имитации
	    unsigned PmodT     :1;  // признак модуляции Т
	    unsigned PmodTau   :1;  // признак модуляции Тау
        unsigned B		   :1;  /*Вид сигнала (при tau > 400 мкс B = 1, иначе B = 0)*/
        unsigned H		   :1;  /*Признак наложения импульсов*/
        unsigned ModF      :1;  /*Признак модуляции по девиации (введен мною)*/
	    unsigned NTacan    :18; //
		union anonymous_union1 {
		int Sector;
		int PostID;
		} au1;
		float Aimp;             // Амплитуда импульса Дб
	    float sigmataui;        // СКО длительности импульса мкс
	    float sigmaT;           // СКО периода повторения импульсов мкс
	    float sigmafc;          // СКО несущей частоты
    } as1;

    struct anonymous_struct2 {                    // SIF (данные пассивной разностно дальномерной системы)
	    unsigned int uDummyS;		// пусто
	    int Sector;
	    float Aimp;             // Амплитуда импульса Дб
        unsigned D4_C1	   :12; /*Позиционный код посылки*/
        unsigned UBD	   :17; /*Позиционный код режима УВД-М*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (3)*/
   /*Слово 1*/
        unsigned DP1	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (4)*/
   /*Слово 2*/
        unsigned DP2	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (5)*/
   /*Слово 3*/
        unsigned DP3	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (6)*/
   /*Слово 4*/
	    unsigned int Rezhim;   // режима SIF
    } as2;

    struct anonymous_struct3 {						// данные активной системы
	    unsigned short uDummyA;		// пусто
	    unsigned short uFlags;		// пусто
	    float fR, fBeta, fEps, fVdop;       // дальность, азимут, угол места, радиальная скорость
	    float fDR, fDBeta, fDEps, fDVdop;   // соответствующие сигмы ошибок этих велечин
        float fAmp;
        BYTE bBeem;
        BYTE bSId;
        BYTE bPId;
        BYTE bCId;
        DWORD dwDopMask;
	    float DummyA[5];        // период повторения импульсов мкс (для имитатора)
    } as3;
} POIT_EXINFO, * PPOIT_EXINFO;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 417 =======================
pattern = """\
typedef struct RXINFO_
{   unsigned short uMinuIndx, uSubtIndx;  // Гео координаты постов (ссылки к blh)
    int iIndex;             // номер имп. в пачке от 0
    unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
	union {
	double Az;				// Азимут а радианах
	double D;				// рх в м
	};
	union {
	double sAz;				// СКО фзимута в рад
	double sD;				// СКО рх в м
	};
    double dTau, dF;        // Длит имп мкс; частота Мгц
} RXINFO, * PRXINFO;
"""

replacement = """\
typedef struct RXINFO_
{   unsigned short uMinuIndx, uSubtIndx;  // Гео координаты постов (ссылки к blh)
    int iIndex;             // номер имп. в пачке от 0
    unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
	union {
	double Az;				// Азимут а радианах
	double D;				// рх в м
	};
	union {
	double sAz;				// СКО фзимута в рад
	double sD;				// СКО рх в м
	};
    double dTau, dF;        // Длит имп мкс; частота Мгц
} RXINFO, * PRXINFO;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 434 =======================
pattern = """\
typedef struct POIT_
{   union {
	FILETIME ftTlock;				//Время локации(100ns ticks)
    unsigned __int64 uqTlock;
	};
    POIT_EXINFO exi;
	union {
	LBH lbh;
	BLH blh[8];                     // 0 - ViewPoint , радианы радианы км
	};                              // 0 for KT - Main Point , радианы радианы км
    unsigned long uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
	int Count;                      // для активной станции 0
    RXINFO rx[];
} POIT, * PPOIT;
"""

replacement = """\
typedef struct POIT_
{
    unsigned __int64 uqTlock;       //Время локации(100ns ticks)
    POIT_EXINFO exi;
	union {
	LBH lbh;
	BLH blh[8];                     // 0 - ViewPoint , радианы радианы км
	};                              // 0 for KT - Main Point , радианы радианы км
    unsigned int uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
	int Count;                      // для активной станции 0
    RXINFO rx[];
} POIT, * PPOIT;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 469 =======================
pattern = """\
typedef struct RXINFOE_
{   unsigned short uMinuIndx, uSubtIndx;  // Гео координаты постов (ссылки к blh)
    int iIndex;             // номер имп. в пачке от 0
    unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
	union {
	double Az;				// Азимут а радианах
	double D;				// рх в м
	};
	union {
	double sAz;				// СКО азимута в рад
	double sD;				// СКО рх в м
	};
	double dAmp;			// Амплитуда дБ 
	double dTau;			// Длит имп мкс
	double dF;				// Цент. частота Мгц
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// период в пачке в нс
	USHORT usImpCount;		// число имп. в пачке
	short sFreq0, sFreq1;	// KHz от центральной
	USHORT usStrobE;
	union {
	struct {
		DWORD D4_C1_UBD;
		DWORD DP1;
		DWORD DP2;
		DWORD DP3; };
	BYTE bModeSData[16]; }; 
	DWORD dwSifMode;
	int iNTacan;
} RXINFOE, * PRXINFOE;
"""

replacement = """\
typedef struct RXINFOE_
{   unsigned short uMinuIndx, uSubtIndx;  // Гео координаты постов (ссылки к blh)
    int iIndex;             // номер имп. в пачке от 0
    unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
	union {
	double Az;				// Азимут а радианах
	double D;				// рх в м
	};
	union {
	double sAz;				// СКО азимута в рад
	double sD;				// СКО рх в м
	};
	double dAmp;			// Амплитуда дБ 
	double dTau;			// Длит имп мкс
	double dF;				// Цент. частота Мгц
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// период в пачке в нс
	USHORT usImpCount;		// число имп. в пачке
	short sFreq0, sFreq1;	// KHz от центральной
	USHORT usStrobE;
	union {
	struct {
		DWORD D4_C1_UBD;
		DWORD DP1;
		DWORD DP2;
		DWORD DP3; };
	BYTE bModeSData[16]; }; 
	DWORD dwSifMode;
	int iNTacan;
} RXINFOE, * PRXINFOE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 503 =======================
pattern = """\
typedef struct POITE_
{   union {
	FILETIME ftTlock;				//Время локации(100ns ticks)
    unsigned __int64 uqTlock;
	};
	int iSector;
	int iPostID;
	int iSmodeAdr;
	int iReserved;
	char bSmodeCall[8];
	DWORD dwReserved[5];
	union {
	LBH lbh;
	BLH blh[9];                     // 0 - ViewPoint , радианы радианы км
	};                              // 0 for KT - Main Point , радианы радианы км
    unsigned long uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
	int Count;                      // для активной станции 0
    RXINFOE rx[];
} POITE, * PPOITE;
"""

replacement = """\
typedef struct POITE_
{
    unsigned __int64 uqTlock;       // Время локации(100ns ticks)
	int iSector;
	int iPostID;
	int iSmodeAdr;
	int iReserved;
	char bSmodeCall[8];
	DWORD dwReserved[5];
	union {
	LBH lbh;
	BLH blh[9];                     // 0 - ViewPoint , радианы радианы км
	};                              // 0 for KT - Main Point , радианы радианы км
    unsigned int uFlags;            // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
	int Count;                      // для активной станции 0
    RXINFOE rx[];
} POITE, * PPOITE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 536 =======================
pattern = """\
typedef struct IMPINFO_
{   long uT;				// uT - Временное смещение от начала СЛ в нс
	DWORD dwFlags;			// FLAG_IMPULS_XXXX
	int iAmp;				// 0 - 32000
	int iDur;				// 10ns кванты
	int iFreq;				// KHz
	short sFreq0, sFreq1;	// KHz от центральной
	DWORD dwFrSigma2;
	DWORD dwPeriod;			// период в нс
	int icount;				// число импульсов в пачке
	union {
		struct {
		DWORD dwD4_C1;
		BYTE bModeSData[16];
		int iModeSDataCount;
		};
		float fMathStat[6];
	};
	short sForInernalUse;
	short sComboPointsCount;
	USHORT usBandCode;
	USHORT usNCOCode;
	USHORT usFrequCode;
	USHORT usFlags;					// флаги CHAN_FLAG_XXXXX
	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
	USHORT usStrobE;
	BYTE bChannelId;	
	BYTE bProcId;	
	BYTE bBoardId;	
	BYTE bMaster;
	DWORD dwStrobEQuad;
	int iPeleng;			// peleng estimation in 1.e-3 parts of degreeee
	/*COMBOPOINTS como;*/	// comented for compiling
} IMPINFO, * PIMPINFO;
"""

replacement = """\
typedef struct IMPINFO_
{   int uT;				// uT - Временное смещение от начала СЛ в нс
	DWORD dwFlags;			// FLAG_IMPULS_XXXX
	int iAmp;				// 0 - 32000
	int iDur;				// 10ns кванты
	int iFreq;				// KHz
	short sFreq0, sFreq1;	// KHz от центральной
	DWORD dwFrSigma2;
	DWORD dwPeriod;			// период в нс
	int icount;				// число импульсов в пачке
	union {
		struct {
		DWORD dwD4_C1;
		BYTE bModeSData[16];
		int iModeSDataCount;
		};
		float fMathStat[6];
	};
	short sForInernalUse;
	short sComboPointsCount;
	USHORT usBandCode;
	USHORT usNCOCode;
	USHORT usFrequCode;
	USHORT usFlags;					// флаги CHAN_FLAG_XXXXX
	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
	USHORT usStrobE;
	BYTE bChannelId;	
	BYTE bProcId;	
	BYTE bBoardId;	
	BYTE bMaster;
	DWORD dwStrobEQuad;
	int iPeleng;			// peleng estimation in 1.e-3 parts of degreeee
	/*COMBOPOINTS como;*/	// comented for compiling
} IMPINFO, * PIMPINFO;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 585 =======================
pattern = """\
typedef struct IMPINFOE_
{   long uT;				// uT - Временное смещение от начала СЛ в нс
	int iDur;				// 10ns кванты
	union {
		char data[20];
		DWORD dwD4_C1;
		float fMathStat[5];
		struct {
		BYTE bModeSData[16];
		int iModeSDataCount;
		};
		struct {
		int iFreq;				// KHz
		short sFreq0, sFreq1;	// KHz от центральной
		DWORD dwFrSigma2;
		DWORD dwPeriod;			// период в нс
		USHORT usPackCount;		// число импульсов в пачке
		USHORT usAtt;
		};
	};
	USHORT usAmp;				// 0 - 32000
	USHORT usTrashHold;
	int iPeleng;				// peleng estimation in 1 / 0x10000 parts of 2*PI
	BYTE bImpFlags;				// FLAG_IMPULS_XXXX
	BYTE bChannelNum;	
	BYTE bChannelFlags;			// флаги CHAN_FLAG_XXXXX
	BYTE bReserved;	
} IMPINFOE, * PIMPINFOE;
"""

replacement = """\
typedef struct IMPINFOE_
{   int uT;				// uT - Временное смещение от начала СЛ в нс
	int iDur;				// 10ns кванты
	union {
		char data[20];
		DWORD dwD4_C1;
		float fMathStat[5];
		struct {
		BYTE bModeSData[16];
		int iModeSDataCount;
		};
		struct {
		int iFreq;				// KHz
		short sFreq0, sFreq1;	// KHz от центральной
		DWORD dwFrSigma2;
		DWORD dwPeriod;			// период в нс
		USHORT usPackCount;		// число импульсов в пачке
		USHORT usAtt;
		};
	};
	USHORT usAmp;				// 0 - 32000
	USHORT usTrashHold;
	int iPeleng;				// peleng estimation in 1 / 0x10000 parts of 2*PI
	BYTE bImpFlags;				// FLAG_IMPULS_XXXX
	BYTE bChannelNum;	
	BYTE bChannelFlags;			// флаги CHAN_FLAG_XXXXX
	BYTE bReserved;	
} IMPINFOE, * PIMPINFOE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 617 =======================
pattern = """\
typedef struct POSTT_
{	union {
	FILETIME ftTlock;				//Время локации(100ns ticks)
	unsigned __int64 uqTlock;
	};
	DWORD dwDuration;               // длительность (100ns ticks)
	int iPosId;
	DWORD dwFlags;                  // флаги FLAG_XXX_XXXX
	DWORD dwKuCode;					// флаги FLAG_KUCODE_XXXX
	DWORD dwKuAttCode;				
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	DWORD dwFreq0;
	DWORD dwFreq1;
	DWORD dwChannelMask;
	USHORT usPostMask;				// вставляется в АДУ инфа о падсоединенных постах 
	USHORT usIndex;	
	int iImpCount;
	DWORD dwReserved[2];
	IMPINFO imp[];
} POSTT, * PPOSTT;
"""

replacement = """\
typedef struct POSTT_
{
	unsigned __int64 uqTlock;       // Время локации(100ns ticks)
	DWORD dwDuration;               // длительность (100ns ticks)
	int iPosId;
	DWORD dwFlags;                  // флаги FLAG_XXX_XXXX
	DWORD dwKuCode;					// флаги FLAG_KUCODE_XXXX
	DWORD dwKuAttCode;				
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	DWORD dwFreq0;
	DWORD dwFreq1;
	DWORD dwChannelMask;
	USHORT usPostMask;				// вставляется в АДУ инфа о падсоединенных постах 
	USHORT usIndex;	
	int iImpCount;
	DWORD dwReserved[2];
	IMPINFO imp[];
} POSTT, * PPOSTT;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 639 =======================
pattern = """\
typedef struct POSTTE_
{	union {
	FILETIME ftTlock;				//Время локации(100ns ticks)
	unsigned __int64 uqTlock;
	};
	DWORD dwChannelMask;
	USHORT usImpCount;
	BYTE bPosId;
	BYTE bIndex;
	IMPINFOE imp[];
} POSTTE, * PPOSTTE;
"""

replacement = """\
typedef struct POSTTE_
{
	unsigned __int64 uqTlock;       // Время локации(100ns ticks)
	DWORD dwChannelMask;
	USHORT usImpCount;
	BYTE bPosId;
	BYTE bIndex;
	IMPINFOE imp[];
} POSTTE, * PPOSTTE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 655 =======================
pattern = """\
typedef struct TRACELISTENTRY_
{   union {
	FILETIME ftStartTime;				//Время создания(100ns ticks)
    unsigned __int64 uqStartTime;
	};
	double dLifeTime;					// in seconds from uqStartTime
    double dEpsylon;					// Здоровье
    int iId;
	int iType;
    int iIll;							// состояние
    int iCount, iPackCount, iRlCount;
	int iBaseCount[5];
	int iSmodeAdr;
	char bSmodeCall[8];
} TRACELISTENTRY, * PTRACELISTENTRY;
"""

replacement = """\
typedef struct TRACELISTENTRY_
{
    unsigned __int64 uqStartTime;       // Время создания(100ns ticks)
	double dLifeTime;					// in seconds from uqStartTime
    double dEpsylon;					// Здоровье
    int iId;
	int iType;
    int iIll;							// состояние
    int iCount, iPackCount, iRlCount;
	int iBaseCount[5];
	int iSmodeAdr;
	char bSmodeCall[8];
} TRACELISTENTRY, * PTRACELISTENTRY;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 674 =======================
pattern = """\
typedef struct RXENTRY_
{   unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
    unsigned long uBasesCode;
	float fTau, fF;         // Длит имп мкс; частота Мгц
} RXENTRY, * PRXENTRY;
"""

replacement = """\
typedef struct RXENTRY_
{   unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
    unsigned int uBasesCode;
	float fTau, fF;         // Длит имп мкс; частота Мгц
} RXENTRY, * PRXENTRY;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 682 =======================
pattern = """\
typedef struct TRACEINFOENTRY_
{   union {
	FILETIME ftLocTime;				//Время локации (100ns ticks)(UTC)
    unsigned __int64 uqLocTime;
	};
	union {
	struct {                    // RL,TACAN (данные пассивной разностно дальномерной системы)
	    unsigned Dummy0    :3;  // пусто
	    unsigned Mod       :3;  // код модуляции (1-ФКМ,2-ЧМ-,4-ЧМ+)
	    unsigned Pol       :2;  // код поляризации
	    unsigned Pim       :1;  // признак имитации
	    unsigned PmodT     :1;  // признак модуляции Т
	    unsigned PmodTau   :1;  // признак модуляции Тау
        unsigned B		   :1;  /*Вид сигнала (при tau > 400 мкс B = 1, иначе B = 0)*/
        unsigned H		   :1;  /*Признак наложения импульсов*/
        unsigned ModF      :1;  /*Признак модуляции по девиации (введен мною)*/
	    unsigned NTacan    :18; //
	    int Sector;
		float Aimp;             // Амплитуда импульса Дб
	    float sigmataui;        // СКО длительности импульса мкс
	    float sigmaT;           // СКО периода повторения импульсов мкс
	    float sigmafc;          // СКО несущей частоты
		};
    struct {                    // SIF (данные пассивной разностно дальномерной системы)
	    unsigned long uSifFlags;   // режима SIF
	    int Sector;
	    float Aimp;             // Амплитуда импульса Дб
        unsigned D4_C1	   :12; /*Позиционный код посылки*/
        unsigned UBD	   :17; /*Позиционный код режима УВД-М*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (3)*/
   /*Слово 1*/
        unsigned DP1	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (4)*/
   /*Слово 2*/
        unsigned DP2	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (5)*/
   /*Слово 3*/
        unsigned DP3	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (6)*/
   /*Слово 4*/
		};
	};
    unsigned long uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
    int RxCount;                    // для активной станции 0
    RXENTRY rx[];
} TRACEINFOENTRY, * PTRACEINFOENTRY;
"""

replacement = """\
typedef struct TRACEINFOENTRY_
{
    unsigned __int64 uqLocTime; // Время локации (100ns ticks)(UTC)
	union anonymous_union2 {
	struct anonymous_struct1 {                    // RL,TACAN (данные пассивной разностно дальномерной системы)
	    unsigned Dummy0    :3;  // пусто
	    unsigned Mod       :3;  // код модуляции (1-ФКМ,2-ЧМ-,4-ЧМ+)
	    unsigned Pol       :2;  // код поляризации
	    unsigned Pim       :1;  // признак имитации
	    unsigned PmodT     :1;  // признак модуляции Т
	    unsigned PmodTau   :1;  // признак модуляции Тау
        unsigned B		   :1;  /*Вид сигнала (при tau > 400 мкс B = 1, иначе B = 0)*/
        unsigned H		   :1;  /*Признак наложения импульсов*/
        unsigned ModF      :1;  /*Признак модуляции по девиации (введен мною)*/
	    unsigned NTacan    :18; //
	    int Sector;
		float Aimp;             // Амплитуда импульса Дб
	    float sigmataui;        // СКО длительности импульса мкс
	    float sigmaT;           // СКО периода повторения импульсов мкс
	    float sigmafc;          // СКО несущей частоты
		} as1;
    struct anonymous_struct2 {                    // SIF (данные пассивной разностно дальномерной системы)
	    unsigned int uSifFlags;   // режима SIF
	    int Sector;
	    float Aimp;             // Амплитуда импульса Дб
        unsigned D4_C1	   :12; /*Позиционный код посылки*/
        unsigned UBD	   :17; /*Позиционный код режима УВД-М*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (3)*/
   /*Слово 1*/
        unsigned DP1	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (4)*/
   /*Слово 2*/
        unsigned DP2	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (5)*/
   /*Слово 3*/
        unsigned DP3	   :29; /*Позиционный код режима УВД-S*/
        unsigned		   :3;  /*Номер слова в кодограмме посылки (6)*/
   /*Слово 4*/
		} as2;
	} au2;
    unsigned int uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
    int RxCount;                    // для активной станции 0
    RXENTRY rx[];
} TRACEINFOENTRY, * PTRACEINFOENTRY;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 731 =======================
pattern = """\
typedef struct RXENTRYE_
{   unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
    unsigned long uBasesCode;
	float fTau, fF;         // Длит имп мкс; частота Мгц
	float fAmp;
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// период в пачке в нс
	USHORT usImpCount;		// число имп. в пачке
	short sFreq0, sFreq1;	// KHz от центральной
	USHORT usStrobE;
} RXENTRYE, * PRXENTRYE;
"""

replacement = """\
typedef struct RXENTRYE_
{   unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
    unsigned int uBasesCode;
	float fTau, fF;         // Длит имп мкс; частота Мгц
	float fAmp;
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// период в пачке в нс
	USHORT usImpCount;		// число имп. в пачке
	short sFreq0, sFreq1;	// KHz от центральной
	USHORT usStrobE;
} RXENTRYE, * PRXENTRYE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 746 =======================
pattern = """\
typedef struct TRACEINFOENTRYE_
{   union {
	FILETIME ftLocTime;				//Время локации (100ns ticks)(UTC)
    unsigned __int64 uqLocTime;
	};
	int iSector;
	int iPostID;
	union {
	struct {
		DWORD D4_C1_UBD;
		DWORD DP1;
		DWORD DP2;
		DWORD DP3; };
	BYTE bModeSData[16]; }; 
	DWORD dwSifMode;
	int iNTacan;
	int iSmodeAdr;
	DWORD dwReserved;
	char bSmodeCall[8];
	unsigned long uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
    int RxCount;                    // для активной станции 0
    RXENTRYE rx[];
} TRACEINFOENTRYE, * PTRACEINFOENTRYE;
"""

replacement = """\
typedef struct TRACEINFOENTRYE_
{
    unsigned __int64 uqLocTime;     // Время локации (100ns ticks)(UTC)
	int iSector;
	int iPostID;
	union anonymouse_union2 {
	struct anonymous_struct1 {
		DWORD D4_C1_UBD;
		DWORD DP1;
		DWORD DP2;
		DWORD DP3; } as1;
	BYTE bModeSData[16]; } au2; 
	DWORD dwSifMode;
	int iNTacan;
	int iSmodeAdr;
	DWORD dwReserved;
	char bSmodeCall[8];
	unsigned int uFlags;           // POIT_FLAGS_CHNL_ХХХ | POIT_FLAGS_ХХХХ
    int RxCount;                    // для активной станции 0
    RXENTRYE rx[];
} TRACEINFOENTRYE, * PTRACEINFOENTRYE;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 787 =======================
pattern = """\
typedef struct SCOPE16C_
{   union {
	FILETIME ftTlock;				// Время локации(100ns ticks)
    unsigned __int64 uqTlock;
	};
	int iPosId;
	int iChanId;
	int iCount;						// число комплексных отсчетов ( sizeof(sData) / 4)
	int iTimeInc;					// Период дискретизации ns
	int iFrequency;					// в КГц
	int iReserved[6];
	short sData[];
} SCOPE16C, * PSCOPE16C;
"""

replacement = """\
typedef struct SCOPE16C_
{
    unsigned __int64 uqTlock; 		// Время локации(100ns ticks)
	int iPosId;
	int iChanId;
	int iCount;						// число комплексных отсчетов ( sizeof(sData) / 4)
	int iTimeInc;					// Период дискретизации ns
	int iFrequency;					// в КГц
	int iReserved[6];
	short sData[];
} SCOPE16C, * PSCOPE16C;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 803 =======================
pattern = """\
typedef struct BASECORRELATION_
{   union {
	FILETIME ftTlock;				// Время локации(100ns ticks)
    unsigned __int64 uqTlock;
	};
	int iMinuId;
	int iSubtId;
	int iChanId;
	int iCount;						// число отсчетов
	int iReserved[8];
	struct DATA
	{	float fRx;
		float fAmp;
	} data[];
} BASECORRELATION, * PBASECORRELATION;
"""

replacement = """\
typedef struct BASECORRELATION_
{
    unsigned __int64 uqTlock;		// Время локации(100ns ticks)
	int iMinuId;
	int iSubtId;
	int iChanId;
	int iCount;						// число отсчетов
	int iReserved[8];
	struct DATA
	{	float fRx;
		float fAmp;
	} data[];
} BASECORRELATION, * PBASECORRELATION;
"""

codograms_changes.append({"pattern": pattern, "replacement": replacement})

#========================================================================
#
#   define the required replacements in qavtctrl.h
#
#========================================================================
qavtctrl_changes = [] 

#======================= @line 12 =======================
pattern = """\
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#else	// DSP_BIOS
#define wchar_t short
#define BYTE unsigned char
#define USHORT unsigned short
#define DWORD unsigned int
#define __int64 long long
#endif	// DSP_BIOS

"""

replacement = """\
#if MSVC
#ifndef __linux
     #include <windows.h>
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#else	// DSP_BIOS
#define wchar_t short
#define BYTE unsigned char
#define USHORT unsigned short
#define DWORD unsigned int
#define __int64 long long
#endif	// DSP_BIOS

#if __GNUC__
#ifdef __linux
    #define BYTE unsigned char
    #define USHORT unsigned short
    #define DWORD unsigned int
    #define PDWORD unsigned int*
    #define __int64 long long
#endif
#endif

"""

qavtctrl_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line  39 =======================
pattern = """\
typedef struct MAINCTRL_O_
{   DWORD dwFlags;                  // флаги O_FLAG_XXXXX
    wchar_t sName[48];              // имя режима
	struct S_ {						// массив стробов (макс 32) для данного режима
	DWORD dwFlags;					// флаги O_FLAG_S_XXXXX
	BYTE bActionID, bCallID, bAcmCode, bUVSMask;
	DWORD dwTimeIncrim;				// прибавка к счетчику времени в 100 нс квантах (100 mks for timer)
	DWORD dwDuration;				// длительность в 100 нс квантах
#ifndef DSP_BIOS
	union {
		struct {
		USHORT usWbFrequ0;
		short sFrequ0;
		};
		DWORD dwFrequ0;
	};
	union {
		struct {
		USHORT usWbFrequ1;
		short sFrequ1;
		};
		DWORD dwFrequ1;
	};
#else
		DWORD dwFrequ0;
		DWORD dwFrequ1;
#endif
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	USHORT usCount;
	USHORT usCountIndx;
	DWORD dwParam;
	DWORD dwCondition;
	BYTE bFrequIndx0;
	BYTE bFrequIndx1;
	BYTE bFrequIndxEx0;
	BYTE bFrequIndxEx1;
	DWORD dwKuCode;
	BYTE bDurIndx;
	BYTE bTIncIndx;
	BYTE bReserved[2];
	DWORD dwReserved[2];
	} s[32];
    int iSNum;						// число сигналов в этом режиме

	int iOCou;                      // сколько раз дкржать этот режим
    int iONext;                     // на какой режим перейти потом
} MAINCTRL_O, * PMAINCTRL_O;
"""

replacement = """\
typedef struct MAINCTRL_O_
{   DWORD dwFlags;                  // флаги O_FLAG_XXXXX
    unsigned short sName[48];              // имя режима
	struct S_ {						// массив стробов (макс 32) для данного режима
	DWORD dwFlags;					// флаги O_FLAG_S_XXXXX
	BYTE bActionID, bCallID, bAcmCode, bUVSMask;
	DWORD dwTimeIncrim;				// прибавка к счетчику времени в 100 нс квантах (100 mks for timer)
	DWORD dwDuration;				// длительность в 100 нс квантах
#ifndef DSP_BIOS
	union {
		struct {
		USHORT usWbFrequ0;
		short sFrequ0;
		};
		DWORD dwFrequ0;
	};
	union {
		struct {
		USHORT usWbFrequ1;
		short sFrequ1;
		};
		DWORD dwFrequ1;
	};
#else
		DWORD dwFrequ0;
		DWORD dwFrequ1;
#endif
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	USHORT usCount;
	USHORT usCountIndx;
	DWORD dwParam;
	DWORD dwCondition;
	BYTE bFrequIndx0;
	BYTE bFrequIndx1;
	BYTE bFrequIndxEx0;
	BYTE bFrequIndxEx1;
	DWORD dwKuCode;
	BYTE bDurIndx;
	BYTE bTIncIndx;
	BYTE bReserved[2];
	DWORD dwReserved[2];
	} s[32];
    int iSNum;						// число сигналов в этом режиме

	int iOCou;                      // сколько раз дкржать этот режим
    int iONext;                     // на какой режим перейти потом
} MAINCTRL_O, * PMAINCTRL_O;
"""

qavtctrl_changes.append({"pattern": pattern, "replacement": replacement})

#======================= @line 219 =======================
pattern = """\
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
	wchar_t sName[64];
    int iId;
    unsigned long uFlags;
} GROUNDINFO, * PGROUNDINFO;
"""

replacement = """\
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
    unsigned short sName[64];
    int iId;
    unsigned int uFlags;
} GROUNDINFO, * PGROUNDINFO;
"""

qavtctrl_changes.append({"pattern": pattern, "replacement": replacement})

#========================================================================
#
#   end of file changes.py
#
#========================================================================

