//******************************************************************************
//
//  файл codograms.h - компонент приложения psadurs.exe - модуля
//     управления изделием A, модуля ВОИ и ПОИ Copyright by Tristan 2009-2014
//     описывает кодограммы обмена с РМО, ПОИ и ВОИ
//
//******************************************************************************
#ifndef CodogramsH
#define CodogramsH
#define CODOGRAMS_H_VER_MAJOR 3
#define CODOGRAMS_H_VER_MINOR 5

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200)
#endif

//******************************************************************************
// При обмене по Ps4 каналу каждый буфер идентифицируется типом dwType в
// соответствии с тем что передается. Далее приводятся используемые типы
//
//******************************************************************************
#define  WTYP_ADUINFO			1       // Старая зона(сейчас не используется)
#define  WTYP_ADUINFOEX			2       // Новая зона (массив ZONEINFO), -1 - новая зона
#define  WTYP_VOIINFO			3       // Новый сектор запрета автозахвата посылаю в виде (массив ZONEINFO)
#define  WTYP_POSTINFO			4       // Координаты постов (устаревшая)
#define  WTYP_GROUNDINFO		5       // Координаты наземных средств (устаревшая)
#define  WTYP_CTLINFO			6       // Информация для управления (структура LOCALCTRLSTRUCT)
#define  WTYP_FREQUCHANGE		7       // Информация для управления частотами (структура FREQUTBL)
#define  WTYP_FDSTATUS			8		// фдк статус (структура FD_STATUS)
#define  WTYP_JUSTIFYRX			9       // Информация юстировки (массив double[8])

#define  WTYP_NEWCONNECTION		0xe     // К серверу произошло новое подключение
#define  WTYP_POSTADDR			0x10    // Адреса постов (структура POSTADDRESSES)
#define  WTYP_CONNECTTOSERVER	0x11	// Команда сменить сервер
#define  WTYP_POSTINFOE			0x12    // Координаты постов (структура POSTCORD)
#define  WTYP_GROUNDINFOE		0x13    // Координаты наземных средств (массив GROUNDINFO)
#define  WTYP_CHANGEMODE		0x14    // Команда сменить режим обзора (структура APPLAYLOCALCTRL)
#define  WTYP_LNASTAT			0x15
#define  WTYP_CHANGEMODEEX		0x16    // Команда сменить режим обзора (структура APPLAYLOCALCTRLEX)
#define  WTYP_BALANCETBL		0x17    // BALANCETBLENTRY array

#define  WTYP_REGINFO       500         // Обмен с АДУ (FTABENT) здесь не используется
#define  WTYP_ET            1000        // Кодограмма (массив ETVOI) - трасса
                                        // uFlags - буковка к номеру трассы, высота в км на другой строке,
                                        // на следующей строке скорости в км/c
#define  WTYP_POIT          2000        // Кодограмма (массив POIT),
#define  WTYP_POITE         2001        // Кодограмма (массив POITE),
                                        // крестиком цвет от типа
#define  WTYP_KT            2500        // нет
#define  WTYP_POSTT			2501		// Кодограмма (массив POSTT)	
#define  WTYP_POSTTE		2502		// Кодограмма (массив POSTTE)	
#define  WTYP_CONSOLE       2005        // текст
#define  WTYP_CONSOLEEX     2006        // стилизованнй текст (структура CONSOLEMSG)
#define  WTYP_TRACELIST     2009        // массив структур TRACELISTENTRY 
#define  WTYP_ADUCOM        3000        // команда для модуля управления
#define  WTYP_ADURSTATUS    3005        // статус модуля управления
#define	 WTYP_AVTSTATUS		0xb00		// статус модуля управления (Avtobaza)
#define	 WTYP_UVSSTATUS		0xb01		// статус модуля управления (Avtobaza)
#define  WTYP_VOICOM        4000        // RMOCOMMAND
#define  WTYP_VOIREP        4001        // RMOCOMMAND
#define  WTYP_FRSELECTION	4002        // FRSELECTION
#define  WTYP_TRACEINFO		4009		// TRACEINFOHEADER с TRACEINFOENTRY 
#define  WTYP_TRACEINFOE	4010		// TRACEINFOHEADER с TRACEINFOENTRYE (new style)
#define  WTYP_SCOPEDATA		6000        // осциллограмма
#define  WTYP_SCOPE16C		6001        // осциллограмма (структура SCOPE16C)
#define  WTYP_BASECORR		6010        // корфункция (BASECORRELATION)
#define  WTYP_TBOLTINFO     7100        // информация от Trimble ThuderBolt
#define  WTYP_SOIMESSAGE	30006
#define  WTYPE_RAW1			0x8005		// осциллограмма 1 бит(структура ACMHEADER)

//******************************************************************************
// некоторые полезные структуруи связанные с координатами чего нибудь
//
//******************************************************************************
#ifndef QTRANSFORMER_H
typedef struct BLPOINT_
{   double dLat, dLon;
} BLPOINT, * PBLPOINT;
typedef struct XYPOINT_
{   double dX, dY;
} XYPOINT, * PXYPOINT;
typedef struct XYZ_
{   double dX, dY, dZ;
} XYZ, * PXYZ;
typedef struct XYH_
{   double dX, dY, dHei;
} XYH, * PXYH;
#ifndef RCTRLH
typedef struct BLH_
{   double dLat, dLon, dHei;
} BLH, * PBLH;
#endif
#endif
typedef struct LBH_
{   double dLon, dLat, dHei;
} LBH, * PLBH;
//******************************************************************************
// структура ZONEINFO
// в форме этой структуры хранятся и передаются между модулями различные
// двумерные области (зоны ответственности, сектора запрета и пр.)
// каждая струкрура это многоугольник (максимум 32-вух угольник) в
// геодезических координатах (вершины определены широтой и долготой)
//
//******************************************************************************
#ifndef ZONEINFO
typedef struct ZONEINFO_
{   int iID;                  // номер зоны/сектора
    int iCount;               // число вершин
    BLPOINT pt[32];            // координаты в градусах (широта, долгота)
} ZONEINFO, * PZONEINFO;
#endif
//******************************************************************************
// структура GROUNDINFO
//******************************************************************************
#ifndef RCTRLH
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
    wchar_t sName[64];
    int iId;
    unsigned long uFlags;			// uFlags & 0xff - type, uFlags & ~0xff - GRINF_FLAG_XXX 
} GROUNDINFO, * PGROUNDINFO;
#endif
#define GRINF_FLAG_JUSTIFY 0x100
//******************************************************************************
// структура POSTCORD (WTYP_POSTINFOE)
//******************************************************************************
typedef struct POSTCORD_
{	GROUNDINFO positions[8];	  
	double dViewPointLat;
	double dViewPointLon;
	DWORD dwPosCount;
	struct BASES_
	{   BYTE bMinuIndx, bSubtIndx;
		USHORT usFlags;
		union {
		DWORD dwTimeOut;	// [0] - timeout, [1] - FrGate, [2] - DurGate [3] - fAreEpsilon
		DWORD dwFrGate;
		DWORD dwDurGate;
		float fAreEpsilon;
		};
		double dBaseLen;
	} bases[5];
} POSTCORD, * PPOSTCORD;
//******************************************************************************
//			POSTADDRESSES
//******************************************************************************
typedef struct POSTADDRESSES_
{	DWORD aAddress[8];
	int iPort[8];
	int iLocalId;
	int iSelectedId;
	DWORD dwFlags;						// FLAG_POSTADDR_XXX
	DWORD dwReserved[5];
} POSTADDRESSES, * PPOSTADDRESSES;

#define FLAG_POSTADDR_SERVERS	0x1

//******************************************************************************
//			LOCALCTRLSTRUCT
//******************************************************************************
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
//******************************************************************************
//			APPLAYLOCALCTRL
//******************************************************************************
typedef struct APPLAYLOCALCTRLEX_
{	int iInternalIndx;
	DWORD dwConditionFlags;
	DWORD dwRegisters[8];
	DWORD dwChangeMask;			// 0x1 Register 0 - valid, ... 0x10 Reg 4 - valid,
								// 0x100 - prog indx - valid, 0x200 cnd flags - valid
} APPLAYLOCALCTRLEX, * PAPPLAYLOCALCTRLEX;

typedef struct APPLAYLOCALCTRL_
{	int iInternalIndx;
	DWORD dwConditionFlags;
	DWORD dwRegisters[4];
	DWORD dwReserved[2];
} APPLAYLOCALCTRL, * PAPPLAYLOCALCTRL;
//******************************************************************************
//				FREQUTBL
//******************************************************************************
typedef struct FREQUTBL_
{	int iCount;
	int iStartIndx;
	union {
		struct {
		short sFrequHb;				// частота настройки WBR (2 - 18 ГГц) в 10 МГц-вых квантах 
		short sFrequLb;				// частота настройки LBR (0.2 - 2 ГГц) в МГц
		};
		DWORD dwFrequ;
	} freq[];
} FREQUTBL, * PFREQUTBL;
//******************************************************************************
// структура CONSOLEMSG
//******************************************************************************
typedef struct CONSOLEMSG_
{   union {
	FILETIME ftTime;				// Время (100ns ticks)
    unsigned __int64 uqTime;
	};
	int iId;
	wchar_t sMessage[];
} CONSOLEMSG, * PCONSOLEMSG;

//******************************************************************************
// структура NEWCONNECTION (К серверу произошло новое подключение)
//******************************************************************************
typedef struct NEWCONNECTION_		// wtype WTYP_NEWCONNECTION
{   union {
	FILETIME ftTime;				// Время UTC (100ns ticks)
    unsigned __int64 uqTime;
	};
	int iSocket;
	DWORD dwFlags;
} NEWCONNECTION, * PNEWCONNECTION;
//******************************************************************************
//
//******************************************************************************
typedef struct CONNECTTOSERVER_		// wtype WTYP_CONNECTTOSERVER
{	DWORD dwFlags;					// CONNECTTOSERVER_FLAGS_XXX
	DWORD aAddress;
	int	iPort;
	int iIndx;
} CONNECTTOSERVER, * PCONNECTTOSERVER;

#define CONNECTTOSERVER_FLAGS_CONNECTED	0x1
//******************************************************************************
//
//******************************************************************************
typedef struct LNASTAT_		// wtype WTYP_
{	DWORD dwFlags;			// 
	DWORD dwReserved[15];
} LNASTAT, * PLNASTAT;

#define LNASTAT_FLAGS_ALL_ON 0x1

//******************************************************************************
//				BALANCETBLENTRY array wType WTYP_BALANCETBL
//******************************************************************************
typedef struct BALANCETBLENTRY_
{	int iFr;
	int iG;
	int iSelector;
	int iFlags;					// BALANCETBL_FLAG_XXX
	BYTE bOutMult[14];
	BYTE bReserved[2];
} BALANCETBLENTRY, * PBALANCETBLENTRY;	

#define	BALANCETBL_FLAG_REQUEST		0x100	
#define	BALANCETBL_FLAG_STORE		0x800


//******************************************************************************
// структура RMOCOMMAND
// Кодограмма команды от РМО
//******************************************************************************
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

#define    VVOD1        1
#define    VVOD2        2
#define    KORR         3
#define    VVODAZ       4
#define    IZMSOPR      5
#define    SBROSOPR     6
#define    TOGSTATIC    7
#define    SETHEIGHT    8
#define    UPDATEINFO   9
#define    TO2D			10
#define    TO3D			11
#define    JUSTIFY		12
#define    REMZRAZ		14
#define    TRACEINFOREQ 20
#define    VKLRMO		67
#define    VIKLRMO		68
#define    REGCHAGE		80
#define    EMIENAB		81
//******************************************************************************
// структура FRSELECTION
// Кодограмма команды от РМО
//******************************************************************************
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

#define    FRSEL_FLAG_TA		0x1
#define    FRSEL_FLAG_CLEARALL	0x2
//******************************************************************************
// структура ETVOI
// Кодограмма экстраполяционной точки (ЭТ) - выход ВОИ
//******************************************************************************
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

#define  ET_FLAGS_NEW           0x001     // новая трасса
#define  ET_FLAGS_AUTO          0x002     // сопровождкние
#define  ET_FLAGS_CANCELED      0x004     // сброшенная
#define  ET_FLAGS_GROUND        0x008     // наземный обект
#define  ET_FLAGS_STATIC        0x010     // неподвижная трасса
#define  ET_FLAGS_IMIT	        0x020     // имитированный обект
#define  ET_FLAGS_AZIMUTH	    0x040     // азимут валидный
#define  ET_FLAGS_2D			0x100     // 2D фильтер
#define  ET_FLAGS_3D			0x200     // 3D фильтер
#define  ET_FLAGS_SMODADR		0x1000    // S-mode bort adress valid
#define  ET_FLAGS_SMODCS		0x2000    // S-mode call sign valid


//******************************************************************************
// структура POIT_EXINFO
//
//******************************************************************************
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
//******************************************************************************
// структура RXINFO
//
//******************************************************************************
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
//******************************************************************************
// структура POIT
//******************************************************************************
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

#define  POIT_FLAGS_CHNL_RL     0
#define  POIT_FLAGS_CHNL_KK     1
#define  POIT_FLAGS_CHNL_TAC    2
#define  POIT_FLAGS_CHNL_SIF    3

#define  POIT_FLAGS_MARKER			0x800
#define  POIT_FLAGS_BLH_VALID		0x1000
#define  POIT_FLAGS_RX_VALID		0x2000
#define  POIT_FLAGS_INF_VALID		0x4000
#define  POIT_FLAGS_VIEWP_VALID		0x8000
#define  POIT_FLAGS_AZIMU_VALID		0x10000
#define  POIT_FLAGS_SECTOR_VALID	0x20000
#define  POIT_FLAGS_POSTID_VALID	0x40000
#define  POIT_FLAGS_TAGBLH_VALID	0x80000
#define  POIT_FLAGS_SMODADR_VALID	0x100000
#define  POIT_FLAGS_SMODCS_VALID	0x200000
//******************************************************************************
// структура RXINFOE
//
//******************************************************************************
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
	USHORT usSifMode;
	USHORT usFlags;
	int iNTacan;
} RXINFOE, * PRXINFOE;
//******************************************************************************
// структура POITE
//******************************************************************************
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
//******************************************************************************
// структура IMPINFO
//
//******************************************************************************
#ifndef RCTRLH
typedef struct COMBOPOINTS_
{   unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
	struct {
		short sRe;
		short sIm;
	} point[];	
} COMBOPOINTS, * PCOMBOPOINTS;
#endif

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
	int iPeleng;			// peleng estimation in 1 / 0x10000 parts of degreeee
	/*COMBOPOINTS como;*/	// comented for compiling
} IMPINFO, * PIMPINFO;

#define	FLAG_IMPULS_RL0			0x0
#define	FLAG_IMPULS_SIF_AC		0x1
#define	FLAG_IMPULS_SIF_S		0x2
#define	FLAG_IMPULS_RL1			0x4	
#define	FLAG_IMPULS_CRL			0x8
#define	FLAG_IMHULS_MATHSTAT	0x10
#define	FLAG_IMPULS_WBR			0x40
#define	FLAG_IMPULS_SIF_REQ		0x80
#define	FLAG_IMPULS_PELENG		0x100

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

//******************************************************************************
// структура POSTT
//******************************************************************************
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
	BYTE bIndx;
	BYTE bIndxWba;
	int iImpCount;
	DWORD dwReserved[2];
	IMPINFO imp[];
} POSTT, * PPOSTT;

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

#define	FLAG_KUCODE_REGENAB	0x10000000
//******************************************************************************
// структура TRACELISTENTRY
//******************************************************************************
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

//******************************************************************************
//
//******************************************************************************
typedef struct RXENTRY_
{   unsigned long uT;       // uT - Временное смещение от начала СЛ в нс
    unsigned long uBasesCode;
	float fTau, fF;         // Длит имп мкс; частота Мгц
} RXENTRY, * PRXENTRY;
//******************************************************************************
//
//******************************************************************************
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
//******************************************************************************
//
//******************************************************************************
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
//******************************************************************************
//
//******************************************************************************
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
//******************************************************************************
//
//******************************************************************************
typedef struct TRACEINFOHEADER_
{   int iTeCount;
	unsigned short usId;
	unsigned char bErrCode;			// TRINF_ERR_XXX					
	unsigned char bType;			// 					
	double dTimeFrom, dTimeTo;			//
	/*TRACEINFOENTRY te[]; или TRACEINFOENTRYE te[];*/
} TRACEINFOHEADER, * PTRACEINFOHEADER;

#define TRINF_ERR_OK			0x0
#define TRINF_ERR_TRNOTFOUND	0x1
#define TRINF_ERR_NOINFO		0x2
//******************************************************************************
// структура SCOPE16C
//******************************************************************************
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
//******************************************************************************
// структура BASECORRELATION (w-type WTYP_BASECORR)
//******************************************************************************
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
//******************************************************************************
//					MAINSTATUS
//******************************************************************************
#ifndef RCTRLH
typedef struct ADUCLI_STATUS_
{	unsigned __int64 uqTime;		// Время UTC (100ns ticks)
	double dSncBufStat;				// Запас стробов в сек
	double dCliBufStat;				// Запас стробов в сек
	double dUvsBufStat;				// Запас стробов в сек
	double dAduBufStat;				// Запас стробов в сек
	double dOut0BufStat;			// Запас в сек
	double dOut1BufStat;			// Запас в сек
	double dSpeedIn;				// Поток на прием мбит в сек
	double dSpeedOut;				// Поток на передачу мбит в сек
	double dOutLoad;				// Процент заполтуния выходного буфера
	double dReserved;				//
	DWORD dwFlags;                  // флаги UVS_STAT_FLAG_XXXXX
	DWORD dwErrCount;
	DWORD dwDoneCount;
	DWORD dwSatellCou;
	BYTE bRubStatus[8];
	struct {
		BYTE bId;
		BYTE bLevel;
	} Satilite[24];
	BLH blh;
	DWORD dwClientId;
	double dLastTCorr;
	unsigned __int64 uqLastUpdateTime;		// Время UTC (100ns ticks)
	BYTE bAcmStatus[24];
	DWORD dwChannelMask;
	BYTE bWbr0MagicNumber;
	BYTE bWbr1MagicNumber;
	BYTE bWbaStatus[2];
	DWORD dwAjustStat;
	DWORD dwReserved[2];
} ADUCLI_STATUS, * PADUCLI_STATUS;

typedef struct MAINSTATUS_
{   DWORD dwFlags;						// ADU_STAT_FLAG_XXXXX            
    DWORD dwStatus;						// reserved
	ADUCLI_STATUS cliStatus[8];
	DWORD dwFdCodeH;
	DWORD dwReserved[15];
} MAINSTATUS, * PMAINSTATUS;

#define		ADU_STAT_FLAG_xxx		0x1

#define		UVS_STAT_FLAG_CONNECTED			0x1
#define		UVS_STAT_FLAG_BUS0CONNECTED		0x2
#define		UVS_STAT_FLAG_BUS1CONNECTED		0x4
#define		UVS_STAT_FLAG_SHASCONNECTED		0x8

#define		UVS_STAT_FLAG_WBR0_POWEROK		0x10
#define		UVS_STAT_FLAG_WBR1_POWEROK		0x20
#define		UVS_STAT_FLAG_WBR0_CACH0		0x40
#define		UVS_STAT_FLAG_WBR0_CACH1		0x80
#define		UVS_STAT_FLAG_WBR1_CACH0		0x100
#define		UVS_STAT_FLAG_WBR1_CACH1		0x200

#define		UVS_STAT_FLAG_SNC_SLVPLLOK		0x800
#define		UVS_STAT_FLAG_SNC_PLLOK			0x1000
#define		UVS_STAT_FLAG_SNC_RUBEXCH		0x2000
#define		UVS_STAT_FLAG_SNC_GPSEXCH		0x4000
#define		UVS_STAT_FLAG_SNC_WBREXCH		0x8000			// wide band receiver exchange
#define		UVS_STAT_FLAG_SNC_KUEXCH		0x20000
#define		UVS_STAT_FLAG_SNC_KU0OK			0x40000
#define		UVS_STAT_FLAG_SNC_KU1OK			0x80000

#define		UVS_STAT_FLAG_SNC_RUBSTAT		0x100000
#define		UVS_STAT_FLAG_SNC_GPSSTAT		0x200000
#define		UVS_STAT_FLAG_SNC_RUB400		0x400000
#define		UVS_STAT_FLAG_SNC_TECCLOCK		0x1000000

typedef struct ACMHEADER_ {
	unsigned __int64 uqExecTime;	// Время исполнения UTC (100ns ticks)
	DWORD dwDuration;               // длительность (100ns ticks)
	USHORT usPostId;
	BYTE bIndx;
	BYTE bIndxWba;
	DWORD dwFlags;                  // флаги FLAG_ACMSTROB_XXXX
	DWORD dwKuCode;					// KU_CODE_FLAG_XXX
	DWORD dwKuAttCode;
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	DWORD dwFreq0;
	DWORD dwFreq1;
	DWORD dwChannelMask;
	USHORT usBandCode;
	USHORT usNCOCode;
	USHORT usFrequCode;
	USHORT usFlags;					// флаги CHAN_FLAG_XXXXX
	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
	USHORT usPostMask;				// вставляется в АДУ инфа о падсоединенных постах 
	BYTE bChannelId;	
	BYTE bProcId;	
	BYTE bBoardId;	
	BYTE bMaster;	
	DWORD dwFdCode;					// 
	DWORD dwStrobEQuad;
	DWORD dwStrobCount;
	USHORT usStrobE;
	USHORT usImpCount;
} ACMHEADER, * PACMHEADER;

#endif
//******************************************************************************
//					ФДК STATUS
//******************************************************************************
typedef struct FDSENTRY_
{
	BYTE	bSubSystem;			// FD_SUSBSYSTEM_XXX
	BYTE	bDevice;			// FD_DEVICE_XXX
	USHORT	usStatus;			// status: 0 - Ok, 0x1-0xffff - some problems (FD_STATUS_XXXX)	
} FDSENTRY, * PFDSENTRY;

#define		FD_SUSBSYSTEM_WBR		1
#define		FD_SUSBSYSTEM_BUS		2
#define		FD_SUSBSYSTEM_RDMT1		3
#define		FD_SUSBSYSTEM_RDMT2		4
#define		FD_SUSBSYSTEM_WBA		5

typedef struct FD_STATUS_
{	unsigned __int64 uqTime;		// Время UTC (100ns ticks)
	BYTE	bType;
	BYTE	bProgress;
	USHORT	usStatus;
	USHORT	usRecords;
	BYTE	bReserved;
	BYTE	bPostId;
	FDSENTRY records[];
} FD_STATUS, * PFD_STATUS;

#define		FD_STATUS_OK			0x0
#define		FD_STATUS_UNAVAL		0x1

//******************************************************************************
// SOI message wType WTYP_SOIMESSAGE(30006)
//******************************************************************************

typedef struct SOIMESSAGE_
{
	// quint16 equal quint16 short and garanteed what this type have 2 bytes size on supported platform;
	quint32 sector;
	quint32 azimut;			// current azimut;
	quint32 freq;			// frequency
	quint32 df;				// frequency passband;
	quint32 amp;			// amplitude;
	quint32 impulsTime;		// pulse duration
	quint32 periodTime;		// period duration;
	unsigned __int64 timestamp;
} SOIMESSAGE, * PSOIMESSAGE;

/**************************************************************************//**
*	Константы ФДК
******************************************************************************/
/*! Определения типов контроля */
#define FDC_TYPE_READY		200		//!< контроль готовности
#define FDC_TYPE_FUNC		201		//!< текущий функциональный контроль
#define FDC_TYPE_DIAG		202		//!< функционально-диагностический контроль
#define FDC_TYPE_COMM		203		//!< контроль линии связи
#define FDC_TYPE_NAV		204		//!< контроль навигационной системы

/*! Определения подсистем */
#define FDC_SS_APM			1		//!< АПМ
#define FDC_SS_BUS			2		//!< БУС
#define	FDC_SS_RDMT1		3		//!< РДМТ 1
#define	FDC_SS_RDMT2		4		//!< РДМТ 2
#define	FDC_SS_WBA			5		//!< ШАС

/*! Определения устройств */
#define FDC_DEV_UNKNOWN			0					//<! Устройство не определено

/*! Устройства АПМ */
#define FDC_APM_HB_LNA(r, d)	(1 + r * 8 + d)		/*!< МШУ диапазона 2 — 18 ГГц
													* r - диапазон:
													* 0 - 2...4 ГГц
													* 1 - 4...8 ГГц
													* 2 - 8...12 ГГц
													* 3 - 12...18 ГГц
													* d - грань от 0 до 7
													*/												
#define FDC_APM_HB_IN_SWITCH(r)	(33 + r)			/*!< Входной коммутатор
													* r - диапазон:
													* 0 - 2...4 ГГц
													* 1 - 4...8 ГГц
													* 2 - 8...12 ГГц
													* 3 - 12...18 ГГц
													*/
#define FDC_APM_HB_WBR(n)		(37 + n)			//!< БПУ 0, 1
#define FDC_APM_HB_WBR0			37					//!< БПУ 0
#define FDC_APM_HB_WBR1			38					//!< БПУ 1
#define FDC_APM_HB_OUT_SWITCH	39					//!< Выходной коммутатор верхнего поддиапазона
#define FDC_APM_HB_KU			40					//!< КУ002 верхнего поддиапазона
#define FDC_APM_LB_KU			41					//!< КУ002 нижнего поддиапазона
#define FDC_APM_LB_LNA(r, d)	(43 + r * 8 + d)	/*!< МШУ диапазона 0,2...2 ГГц
													* r - диапазон:
													* 0 - 1...2 ГГц
													* 1 - 0,5...1 ГГц
													* 2 - 0,2...0,5 ГГц
													* d - направление:
													* для нижнего диапазона 0...3
													* для верхних диапазонов 0...7
													*/
#define FDC_APM_LB_SWITCH(n)	(64 + n)			/*!< Коммутаторы нижнего поддиапазона
													* 0 - коммутатор диапазона 0,5...1 ГГц
													* 1 - коммутатор диапазона 1...2 ГГц
													* 2 - выходной коммутатор
													* 3 - коммутатор диапазона 0,8...2 ГГц
													*/

/*! Устройства БУС */
#define FDC_BUS_MODEM(n)		(1 + n)				/*!< Модемы:
													* 0 - левый
													* 1 - центральный
													* 2 - правый
													*/
#define FDC_BUS_RUBIDIUM		4					//!< Стандарт частоты
#define FDC_BUS_UVS				5					//!< УВС
#define FDC_BUS_POWER(n)		(6 + n)				//!< Источник питания 12В 0...1

/*! Устройства РДМТ */
#define FDC_RDMT_CPC			1					//!< CPC600
#define FDC_RDMT_ESW			2					//!< Коммутатор ESW
#define FDC_RDMT_SNC			3					//!< СНЦ003
#define FDC_RDMT_UAP1			4					//!< УАП001 - 1
#define FDC_RDMT_PRU1			5					//!< ПРУ001
#define FDC_RDMT_UDS			6					//!< УДС001
#define FDC_RDMT_PRU2			7					//!< ПРУ001А
#define FDC_RDMT_UAP2			8					//!< УАП001 - 2
#define FDC_RDMT_KSO			9					//!< КСО001
#define FDC_RDMT_RPM			10					//!< РПМ002
#define FDC_RDMT_UAP3			11					//!< УАП001 - 3
#define FDC_RDMT_URP			12					//!< УРП001

/*! Устройства ШАС */
#define FDC_WBA_LL				1					//!< 2ЛЛ01
#define FDC_WBA_CP1				2					//!< 2ЦП01 - 1
#define FDC_WBA_CP2				3					//!< 2ЦП01 - 2
#define FDC_WBA_CP(n)			(2 + n)				//!< 2ЦП01 - 0...1
#define FDC_WBA_UU				4					//!< 2УУ01
#define FDC_WBA_HOST			5					//!< host


//-----------------------------------------------------------------------------
//   Конец файла *.h
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif

