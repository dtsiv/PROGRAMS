//******************************************************************************
//
//  файл qrctrl.h - компонент приложения   qavtadu.exe - модуля
//     управления изделием Avtobaza-m Copyright by Tristan 2013
//     описывает главную структуру управления MAINCTRL, кодограммы обмена с СНЦ,
//
//******************************************************************************
#ifndef RCTRLH
#define RCTRLH
#define RCTRLH_VERSION "1.01.07"
#ifndef DSP_BIOS
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200)
#endif

typedef struct BLH_
{   double dLat, dLon, dHei;
} BLH, * PBLH;

//******************************************************************************
//  структура MAINCTRL_O
//  компонент главной структуры управления MAINCTRL
//  отвечающий за программу обзора
//
//******************************************************************************
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

typedef struct MAINCTRL_OC_
{
	char sConventions[256];			// строка условий

} MAINCTRL_OC, * PMAINCTRL_OC;

#ifndef DSP_BIOS
typedef struct MAINCTRL_O_::S_ SCAN_PROG_S, * PSCAN_PROG_S;
#endif	// DSP_BIOS

#define    O_FLAG_S_CALL			0x1    // это вызов 
#define    O_FLAG_S_ACTION			0x2    // это действие
#define    O_FLAG_S_GOTO			0x4    // это ветвление
#define    O_FLAG_S_FORK			0x8    // это fork ветвление
#define    O_FLAG_S_ENABLE			0x10   // строб разрешен
#define    O_FLAG_S_USEF0			0x20   // использовать код частоты сигнала
#define    O_FLAG_S_USEFI0			0x40   // использовать индекс частоты сигнала
#define    O_FLAG_S_MODFR0			0x80	
#define    O_FLAG_S_MODFRFR0		0x100	
#define    O_FLAG_S_USEF1			0x2000   // использовать код частоты сигнала
#define    O_FLAG_S_USEFI1			0x4000   // использовать индекс частоты сигнала
#define    O_FLAG_S_MODFR1			0x8000	
#define    O_FLAG_S_MODFRFR1		0x10000	
#define    O_FLAG_S_USECNTINDX		0x20000	
#define    O_FLAG_S_IGNORE			0x40000	
#define    O_FLAG_S_INHERITAZ0		0x80000	
#define    O_FLAG_S_INHERITAZ1		0x100000	
#define    O_FLAG_S_INHERITINDX		0x200000	
#define    O_FLAG_S_USECNTFTBLINDX  0x400000	
#define    O_FLAG_S_USEDURFTBLINDX  0x800000	

#define    O_FLAG_LOOP        0x10  // повторять этот режим безконца
#define    O_FLAG_HIDE        0x20  // не показывать этот режим оператору РМО
#define    O_FLAG_START       0x40  // этот режим стартовый

#define    O_ACTION_NON				0		// non
#define    O_ACTION_NOP				1		// nop
#define    O_ACTION_SETTIME			2		// 
#define    O_ACTION_STOP			3		// встать в состояние пауза
#define    O_ACTION_IFADDUVS		4		// 
#define    O_ACTION_IFCNDANY		5		// 
#define    O_ACTION_IFCNDALL		6		// 
#define    O_ACTION_IFNOTCNDANY		7		// 
#define    O_ACTION_IFNOTCNDALL		8		// 
#define    O_ACTION_SETCND			9		// 
#define    O_ACTION_CLEARCND		10		// 
#define    O_ACTION_QUEUETMR		11		// 
#define    O_ACTION_ROUNDTIME		12		// 
#define    O_ACTION_SLEEP			13		// 
#define    O_ACTION_WAITCNDANY		14		// 
#define    O_ACTION_WAITCNDALL		15		// 
#define    O_ACTION_SETSLICE		16
#define    O_ACTION_SWITCHENAB		17		// 
#define    O_ACTION_SWITCHDISNAB	18		// 
#define    O_ACTION_POSTFD			19		// 
#define    O_ACTION_BOOST			20		// 
#define    O_ACTION_SETLNA			21		// 
#define    O_ACTION_SETFD			22		// 
#define    O_ACTION_SETFD_H			23		// 
#define    O_ACTION_SETSTBCOUNT		24		// 
#define    O_ACTION_SETATBL			25		// 
#define    O_ACTION_SETKUSETBITS	26		// 
#define    O_ACTION_SETKUCLRBITS	27		// 

#define    O_CONDITION_RUB400		0x1		// пришел статус 400
#define    O_CONDITION_NEWBUS0		0x2		// BUS 0 got connection
#define    O_CONDITION_NEWBUS1		0x4		// BUS 1 got connection
#define    O_CONDITION_ADJ_FINISED	0x8		// Adjust finished
#define    O_CONDITION_TIMER0		0x10	// time interval 0 expired
#define    O_CONDITION_TIMER1		0x20	// time interval 1 expired
#define    O_CONDITION_TIMER2		0x40	// time interval 2 expired
#define    O_CONDITION_TIMER3		0x80	// time interval 3 expired
#define    O_CONDITION_ADJ_ERROR	0x100	// Adjust got error
//******************************************************************************
//						channel control table
//******************************************************************************
typedef struct CHANTABPARAM_
{	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
} CHANTABPARAM, * PCHANTABPARAM;

typedef struct CHANTABENTRY_
{	USHORT usBandCode;
	USHORT usNCOCode;
	USHORT usFrequCode;
	USHORT usFlags;				// флаги CHAN_FLAG_XXXXX
	CHANTABPARAM p[8];
} CHANTABENTRY, * PCHANTABENTRY;


typedef struct MAINCTRL_C_
{   DWORD dwFlags;                  // флаги C_FLAG_XXXXX
	DWORD dwChanId;					
	CHANTABENTRY c[16];
} MAINCTRL_C, * PMAINCTRL_C;

typedef struct CHANTABPOSTENTRY_
{	USHORT usBandCode;
	USHORT usNCOCode;
	USHORT usFrequCode;
	USHORT usFlags;				// флаги CHAN_FLAG_XXXXX
	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
} CHANTABPOSTENTRY, * PCHANTABPOSTENTRY;

typedef struct CHANNELTBL_
{   DWORD dwFlags;                  // флаги C_FLAG_XXXXX
	DWORD dwPostId;					
	DWORD dwChanId;					
	CHANTABPOSTENTRY c[16];
} CHANNELTBL, * PCHANNELTBL;

#define CHAN_FLAG_RAW16			0x1
#define CHAN_FLAG_IMPU			0x2
#define CHAN_FLAG_SIFDEC		0x4
#define CHAN_FLAG_RAW2			0x8
#define CHAN_FLAG_NO_CHANPROC	0x10
#define CHAN_FLAG_LOG			0x10
#define CHAN_FLAG_BLOCKWINER	0x20
#define CHAN_FLAG_BLOCKPACK		0x40
#define CHAN_FLAG_NOLINKPACK	0x80

//******************************************************************************
//						Rx posts info table
//******************************************************************************
typedef struct GROUNDINFO_
{   BLH blh;
    double dOrientation;
	wchar_t sName[64];
    int iId;
    unsigned long uFlags;
} GROUNDINFO, * PGROUNDINFO;

typedef struct MAINCTRL_P_
{	GROUNDINFO positions[8];	  
	double dViewPointLat;
	double dViewPointLon;
	DWORD dwPosCount;
	struct
	{   BYTE bMinuIndx, bSubtIndx;
		USHORT usFlags;
#ifndef DSP_BIOS
		union {
#else
		union DSP_B {
#endif
		DWORD dwTimeOut;	// [0] - timeout, [1] - FrGate, [2] - DurGate
		DWORD dwFrGate;
		DWORD dwDurGate;
		};
		double dBaseLen;
	} bases[5];

} MAINCTRL_P, * PMAINCTRL_P;
//******************************************************************************
//					WBR attenuation table
//******************************************************************************
typedef struct MAINCTRL_A_
{   DWORD dwFlags;						// SNC_TBL_FLAG_XXX - 1 - use from KU            
    DWORD dwJustCode;
	struct H
	{	USHORT usFreqFrom;
		char cA0, cA1, cB0, cB1;
	} hb[32];
	struct L
	{	USHORT usFreqFrom;
		char cA0, cA1;
	} lb[32];
	struct G
	{	USHORT usFreqFrom;
		char cG[8];
	} g[32];
	struct WBA
	{	DWORD dwWbaFlags;
		DWORD dwWbaDeci;
		USHORT usTr[16];
	} w0[16], w1[16];
} MAINCTRL_A, * PMAINCTRL_A;

#define SNC_TBL_FLAG_USEKU		0x1
//******************************************************************************
//  Собственно сама главная структура управления MAINCTRL
//  отвечает за управление всем, заряжается при старте и сохраняется на выходе в
//  файле конфигурации с именем из проектных параметров (по умолчанию mainctrl.cfg)
//
//******************************************************************************
typedef struct MAINCTRL_
{   DWORD dwFlags;                  // флаги управления MAIN_FLAG_ХХХХ
    DWORD dwOStart;                 // кандидат в стартовые режимы
    DWORD dwONum;                   // число определенных режимов (макс 64)
    MAINCTRL_O o[64];               // массив структур режимов (программа обзора)
	unsigned __int64 quForvardTime;
	DWORD aUvsAddress[8];
	int iUvsPort[8];
	int iLocalUvsId;
	MAINCTRL_C c[24];
	DWORD dwChannelChangeMask[8];
#ifndef DSP_BIOS
	union {
		struct {
		USHORT usWbFrequ;
		short sFrequ;
		};
		DWORD dwFrequ;
	} ftbl[140];
#else
	DWORD dwFrequ[140];
#endif
	MAINCTRL_P p;
	MAINCTRL_A a[8];
	DWORD dwAttChangeMask;
	DWORD dwReserved0;
	DWORD dwRegisters[8];
	BYTE bPostEnableMask;
	BYTE bReserved[3];
	DWORD dwDcRegisters[4];
    MAINCTRL_OC oc[64];
	DWORD dwReserved[10];
} MAINCTRL, * PMAINCTRL;

//Main dispatch flags
#define    MAIN_FLAG_RESTEPROG			0x1				// 
#define    MAIN_FLAG_STOP				0x2				// 
#define    MAIN_FLAG_UVSENAB_CHANGE		0x80
#define    MAIN_FLAG_LNASTAT_CHANGE		0x100
#define    MAIN_FLAG_FREQUTBL_CHANGE	0x200
#define    MAIN_FLAG_RTL_CHANGE			0x400			// запрос на синхронизацию TL
#define    MAIN_FLAG_PAUSE_CHANGE		0x800			// used
#define    MAIN_FLAG_CHANTBL_CHANGE		0x1000			// запрос на chantbl
#define    MAIN_FLAG_ATTTBL_CHANGE		0x2000			// запрос на atttbl
#define    MAIN_FLAG_POSTINFO_CHANGE	0x4000			// запрос на обновление postinfo
#define    MAIN_FLAG_PAUSE				0x8000          // пауза
                                                 //
#define    MAIN_FLAG_SNCCTLF		   0x1000000 // Боевая кодограмма	

//******************************************************************************
//					Кодограммы ADU CTL
//******************************************************************************
#define		ADU_TYPE_STATUS		0xb00
#define		ADU_TYPE_MCTRL		0xb01
#define		ADU_TYPE_TEXT		0xb02
#define		ADU_TYPE_PAUSE		0xb03
#define		ADU_TYPE_RELOAD		0xb04
#define		ADU_TYPE_SLEVEL		0xb05

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
	DWORD dwReserved[16];
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

#define		ADU_STAT_FLAG_xxx		0x1

typedef struct PAUSESTATE_
{   DWORD dwFlags;						// 1 - pause on            
    DWORD dwReserved;
} PAUSESTATE, * PPAUSESTATE;	
#define		ADU_PAUSE_FLAG			0x1

typedef struct SIGNALLEVEL_
{   DWORD dwFlags;						// XXXXXX_FLAG_XXXXX            
    DWORD dwReserved;					// reserved
	double dSigma;
	double dEver;
	double dSigmaA;
	int iPosId; 
	int iChanId;
	int iCount; 
	int iReserved[7];
} SIGNALLEVEL, * PSIGNALLEVEL;
//******************************************************************************
//					Кодограммы СНЦ
//******************************************************************************

#define		SNC_TYPE_COMMAND	0xa000
#define		SNC_TYPE_STROB		0xa000
#define		SNC_TYPE_GETTIME	0xa010
#define		SNC_TYPE_INITATBL	0xa011
#define		SNC_TYPE_STATUS		0xa012	// передается из СНЦ по разным поводам
#define		SNC_TYPE_GPS		0xa021
#define		SNC_TYPE_RUBID		0xa022
#define		ACL_TYPE_STATUS		0xe000
#define		ACL_TYPE_CHANTBL	0xe001
#define		ACL_TYPE_ATTTBL		0xe002
#define		ACL_TYPE_WBRATTTBL	0xe003
#define		ACM_TYPE_STROB		0x8000
#define		ACM_TYPE_CHANTBL	0x8001
#define		ACM_TYPE_STATUS		0x8002
#define		ACM_TYPE_RAW16		0x8003
#define		ACM_TYPE_POSTT		0x8004
#define		ACM_TYPE_RAW1		0x8005
#define		ACM_TYPE_POSTTE		0x8006
#define		ACM_TYPE_REG		0x8345

//******************************************************************************
//					SNC_TYPE_STROB for SNC
//******************************************************************************

typedef struct STROBLONGPARAMSHB_
{	short sTrash[14];
	short sTrash1[14];
	short sTrashWba[16];
	short sTrashWba1[16];
	BYTE bAtt[14];			// KNT att for channels 0, 1, 2, 3, 5, 6, 7, 12, 13, 14, 15, 17, 18, 19
	BYTE bAttWbr[2];
	BYTE bAttWba[2];
	BYTE bWbrOutMult[14];
	BYTE bWbaOutMult[2];
	STROBLONGPARAMSHB_() { initialise(); } 
	void initialise()
	{	int i = 0;
		while(i < 14)
		{	sTrash[i] = 36;
			sTrash1[i] = 28;
			bAtt[i] = 30;
			bWbrOutMult[i] = 100;
			i++;
		}
		i = 0;
		while(i < 16)
		{	sTrashWba[i] = 36;
			sTrashWba1[i] = 28;
			i++;
		}
		bWbaOutMult[0] = 100;
		bWbaOutMult[1] = 100;
		bAttWbr[0] = 20;
		bAttWbr[1] = 20;
		bAttWba[0] = 20;
		bAttWba[1] = 20;
	}
} STROBLONGPARAMSHB, * PSTROBLONGPARAMSHB;

typedef struct STROBLONGPARAMSLB_
{	short sTrashLbr[2];
	short sTrashLbr1[2];
	BYTE bAttL[2];
	BYTE bLbrOutMult[2];
	STROBLONGPARAMSLB_() { initialise(); }
	void initialise()
	{	int i = 0;
		while(i < 2)
		{	sTrashLbr[i] = 36;
			sTrashLbr1[i] = 28;
			bAttL[i] = 20;
			bLbrOutMult[i] = 100;
			i++;
		}
	}
} STROBLONGPARAMSLB, * PSTROBLONGPARAMSLB;

typedef struct SIFLONGPAPAMS_
{
	short sTrashSif[4];			// for channels 10, 11, 22, 23
	short sTrashSif1[4];		// --- " ---
	BYTE bAttSif[4];
	BYTE bSifOutMult[4];
} SIFLONGPAPAMS, *PSIFLONGPAPAMS;

typedef struct SNC_STROB_
{   unsigned __int64 uqExecTime;	// Время исполнения UTC (100ns ticks)
	DWORD dwDuration;               // длительность (100ns ticks)
	DWORD dwPostId;
	DWORD dwFlags;                  // флаги управления SNC_STROB_FLAG_ХХХХ
	DWORD dwAcmCode;
	DWORD dwAzCode0;
	DWORD dwAzCode1;
#ifndef DSP_BIOS
	union {
		struct {
		USHORT usWbFreq0;
		short sFreq0;
		};
		DWORD dwFreq0;
	};
	union {
		struct {
		USHORT usWbFreq1;
		short sFreq1;
		};
		DWORD dwFreq1;
	};
#else
	DWORD dwFreq0;
	DWORD dwFreq1;
#endif
	DWORD dwKuCode;
	DWORD dwKuAttCode;
	DWORD dwFdCode;
	DWORD dwStrobCount;
	DWORD dwComboChanMask;
	DWORD dwComboPointCount;
	short sTrash[14];			// for channels 0, 1, 2, 3, 5, 6, 7, 12, 13, 14, 15, 17, 18, 19
	short sTrash1[14];			// --- " ---
	short sTrashWba[16];
	short sTrashWba1[16];
	short sTrashLbr[6];			// for channels 9, 10, 11, 21, 22, 23
	short sTrashLbr1[6];		// --- " ---
	BYTE bAtt[14];
	BYTE bAttL[6];
	BYTE bAttWbr[2];
	BYTE bAttWba[2];
} SNC_STROB, * PSNC_STROB;

typedef struct SNC_STROBA_
{   DWORD dwSize;					// размер пакета в байтах, по нему можно понять количество стробов
	DWORD dwType;					// SNC_TYPE_STROB
#ifndef DSP_BIOS
	SNC_STROB s[];					// массив стробов
#else
	SNC_STROB s[1];					// массив стробов
#endif
} SNC_STROBA, * PSNC_STROBA;

#define SNC_STROB_FLAG_CENTRAL	0x1

//******************************************************************************
//					SNC_TYPE_STROB exept SNC
//******************************************************************************
typedef struct ADU_STROB_
{   unsigned __int64 uqExecTime;	// Время исполнения UTC (100ns ticks)
	DWORD dwDuration;               // длительность (100ns ticks)
	DWORD dwPostId;
	DWORD dwFlags;                  // флаги управления SNC_STROB_FLAG_ХХХХ
	DWORD dwAcmCode;
	DWORD dwAzCode0;
	DWORD dwAzCode1;
#ifndef DSP_BIOS
	union {
#else
	union DSP_B0 {
#endif
		struct {
		USHORT usWbFreq0;
		short sFreq0;
		};
		DWORD dwFreq0;
	};
#ifndef DSP_BIOS
	union {
#else
	union DSP_B1 {
#endif
		struct {
		USHORT usWbFreq1;
		short sFreq1;
		};
		DWORD dwFreq1;
	};
	DWORD dwKuCode;
	DWORD dwKuAttCode;
	DWORD dwFdCode;
	DWORD dwStrobCount;
	DWORD dwComboChanMask;
	DWORD dwComboPointCount;
} ADU_STROB, * PADU_STROB;

typedef struct ADU_STROBA_
{   DWORD dwSize;					// размер пакета в байтах, по нему можно понять количество стробов
	DWORD dwType;					// SNC_TYPE_STROB
	ADU_STROB s[];					// массив стробов
} ADU_STROBA, * PADU_STROBA;

#define		FDH_HB_NOISE_0			0x1
#define		FDH_HB_NOISE_1			0x2
#define		FDH_LB_NOISE_0			0x4
#define		FDH_LB_NOISE_1			0x8
#define		FDH_DIRECT_CTRL			0x10
#define		FDH_CHAN_OFF			0x20
#define		FDH_CHAN_OSC			0x40
#define		FDH_TIME_INC_OVERRIDE	0x80
#define		FDH_STATIC_THRESH_TBL	0x100
#define		FDH_STATIC_KNTATT_TBL	0x200
#define		FDH_POI_LOG				0x400
#define		FDH_ACM_LOG				0x800
#define		FDH_AJUST				0x1000
#define		FDH_FDK					0x2000
#define		FDH_PELENG_MODE			0x8000
#define		FDH_CHAN_OSC_AF			0x4000
//{ <SMA> добавлены определения
#define		FDH_MATHSTAT			(FDH_AJUST | FDH_FDK)			// биты вычисления статистик
#define		KU_FROM_FDH(x)			((x) << 16)						// получение кода KU из FDH
#define		FDH_FROM_KU(x)			((x) >> 16)						// получение кода FDH из KU
#define		KU_IS_MATHSTAT(x)		(x & KU_FROM_FDH(FDH_MATHSTAT))					// проверка необходимости вычисления статистик
#define		KU_IS_FDK(x)			(KU_IS_MATHSTAT(x) == KU_FROM_FDH(FDH_FDK))		// проверка режима ФДК
#define		KU_IS_AJUST(x)			(KU_IS_MATHSTAT(x) == KU_FROM_FDH(FDH_AJUST))	// проверка режима калибровки
// В логике aduclient и uvsserver статистики вычисляются при ФК и калибровке.
// Калибровка и ФК проводятся только при установке одного из битов
//}

#define		KU_HB_DIRECT			0x1
#define		KU_HB_OUT_1				0x2
#define		KU_HB_OUT_2				0x4
#define		KU_HB_OUT_3				0x8
#define		KU_LB_DIRECT			0x10
#define		KU_LB_OUT				0x20
#define		KU_KRM_0				0x40
#define		KU_KRM_1				0x80
#define		KU_KRM_2				0x100
#define		KU_VDM_0				0x200
#define		KU_VDM_1				0x400
//{ <SMA> добавлены константы 
#define		KU_KRM_MASK				(KU_KRM_0 | KU_KRM_1 | KU_KRM_2)
#define		KU_KRM_SHIFT			6
#define		KU_VDM_MASK				(KU_VDM_0 | KU_VDM_1)
#define		KU_VDM_SHIFT			9
#define		KU_ATT_TBL_MASK			0xF800
#define		KU_ATT_TBL_SHIFT		11
#define		WBR_FROM_FD(x)			(x & 0x003F)
#define		LBR_FROM_FD(x)			((x >> 8) & 0x00FF)
#define		KNT_FROM_FD(x)			((x >> 16) & 0x001F)
#define		WBA_FROM_FD(x)			((x >> 24) & 0x001F)
//}

//******************************************************************************
//					SNC_TYPE_GETTIME
//******************************************************************************
typedef struct SNC_GETTIME_
{   DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;					// SNC_TYPE_GETTIME
    unsigned __int64 uqServerTime;	// Время ADU UTC (100ns ticks)
    unsigned __int64 uqUvsTime;		// Время UVS UTC (100ns ticks)
    unsigned __int64 uqClientTime;	// Время ADUclient UTC (100ns ticks)
    unsigned __int64 uqSNCTime;		// Время SNC UTC (100ns ticks)
	DWORD dwFlags;                  // флаги
	DWORD dwPostId;
} SNC_GETTIME, * PSNC_GETTIME;

#define		GETTIME_FLAG_SRV	0x100
#define		GETTIME_FLAG_RUB	0x200

//******************************************************************************
//					SNC_TYPE_INITATBL
//******************************************************************************
typedef struct SNC_INITATBL_
{   DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;					// SNC_TYPE_INITATBL
	DWORD dwTblId;                  // ID
	DWORD dwFlags;                  // флаги INITATBL_FLAG_XXX
	DWORD dwPostId;
	DWORD dwReserved[11];
} SNC_INITATBL, * PSNC_INITATBL;

#define		INITATBL_FLAG_ALT	0x1
//******************************************************************************
//					SNC_TYPE_STATUS
//******************************************************************************
typedef struct SNC_STATUS_
{	DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;					// SNC_TYPE_STATUS
	unsigned __int64 uqTime;		// Время UTC (100ns ticks)
	DWORD dwFlags;					// флаги FLAG_SNC_STATUS_XXXXX
	DWORD CountErr;
	DWORD CountDone;
	DWORD dwSatellCou;
	char Satellits[6][96];
	BYTE Status_Rub[8];
	char bCoord[64];
	BYTE bMagic0, bMagic1; 
	char  cReserved[6];			// 1 dword
} SNC_STATUS, * PSNC_STATUS;


#define		FLAG_TIME_NOVALID		0x0
#define		FLAG_TIME_SET			0x1
#define		FLAG_TIME_RUBID			0x2
#define		FLAG_TRANSMIT_RUBID		0x4
#define		FLAG_TRANSMIT_GPS		0x8
#define		FLAG_STATUS_PLL			0x10

//******************************************************************************
//					RUBID, GPS command
//******************************************************************************

typedef struct RUB_COM_
{	DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;
	DWORD CountErr;
	DWORD res;
	char  Command[256];
} RUB_COM, *pRUB_COM;

typedef struct GPS_COM_
{	DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;
	DWORD CountErr;
	DWORD res;
	char  Command[256];
} GPS_COM, *pGPS_COM;
//******************************************************************************
//						Exchange with ACM
//******************************************************************************

#define	FLAG_ACMSTATUS_ERROR	0
#define	FLAG_ACMSTATUS_STROB	1
#define	FLAG_ACMSTATUS_UART		2
#define	FLAG_ACMSTATUS_TABL		4

typedef struct ACMSTATUS_
{	DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;					// ACM_TYPE_STATUS
	BYTE bChannelId;	
	BYTE bProcId;	
	BYTE bBoardId;	
	BYTE bMaster;	
	DWORD dwStatus;
	DWORD dwReserved[4];
} ACMSTATUS, * PACMSTATUS;

typedef struct ACMCHANTBL_
{	DWORD dwSize;					// размер пакета в байтах
	DWORD dwType;					// ACM_TYPE_CHANTBL
	DWORD dwFlags;
	BYTE bChannelId;	
	BYTE bProcId;	
	BYTE bBoardId;	
	BYTE bMaster;	
	CHANTABPOSTENTRY c[16];
} ACMCHANTBL, * PACMCHANTBL;
//******************************************************************************
//					ACM_TYPE_STROB
//******************************************************************************
typedef struct COMBOPOINTS_
{   unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
	struct {
		short sRe;
		short sIm;
	} point[];	
} COMBOPOINTS, * PCOMBOPOINTS;

typedef struct ACMIMPULS_
{   unsigned int uT;       // uT - Временное смещение от начала СЛ в нс
	DWORD dwFlags;			// FLAG_IMHULS_XXXX
	int iAmp;				// 0 - 32000
	int iDur;				// 10ns кванты
	int iFreq;				// KHz
	short sFreq0, sFreq1;	// KHz от центральной. для ШАС 10КГц 
	DWORD dwFrSigma2;
	DWORD dwPeriod;			// период в пачке в нс
	int icount;				// число импульсов в пачке
#ifndef DSP_BIOS
	union {
		struct {
#endif	// DSP_BIOS
		DWORD dwC1_D4;		// код SIF
		BYTE bModeSData[16];
		int iModeSDataCount;
#ifndef DSP_BIOS
		};
		float fMathStat[6];
	};
#endif	// DSP_BIOS
	short sForInernalUse;
	short sComboPointsCount;
	/*COMBOPOINTS como;*/ // comented for compiling
} ACMIMPULS, * PACMIMPULS;

#define	FLAG_IMHULS_SIF_AC		0x1
#define	FLAG_IMHULS_SIF_S		0x2
#define	FLAG_IMHULS_MATHSTAT	0x10

typedef struct ACMHEADER_ {
	unsigned __int64 uqExecTime;	// Время исполнения UTC (100ns ticks)
	DWORD dwDuration;               // длительность (100ns ticks)
	USHORT usPostId;
	USHORT usIndx;
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

#define		FLAG_ACMSTROB_LOST			0x10000
#define		FLAG_ACMSTROB_SIFREP		0x1
#define		FLAG_ACMSTROB_SIFREQU		0x2
#define		FLAG_ACMSTROB_RAW16			0x4
#define		FLAG_ACMSTROB_RAW1			0x8
/*
#define		KU_CODE_DIRECT_CTRL			0x00000001
#define		KU_CODE_FLAG_LBOUT0			0x00000002
#define		KU_CODE_FLAG_LBNOISE0		0x00000004
#define		KU_CODE_FLAG_LBNOISE1		0x00000008
#define		KU_CODE_FLAG_HBDIRECT		0x00000010
#define		KU_CODE_FLAG_HBOUT0			0x00000020
#define		KU_CODE_FLAG_HBNOISE0		0x00000040
#define		KU_CODE_FLAG_HBNOISE1		0x00000080
#define		KU_CODE_STATIC_CTRL			0x00000100
#define		KU_CODE_STATIC_THRESH		0x00000200
#define		KU_CODE_STATIC_CTRL_ALT		0x00000400
#define		KU_CODE_OUT2				0x00008000
#define		KU_CODE_VDM0				0x00080000
#define		KU_CODE_VDM1				0x00100000
#define		KU_CODE_DIAG				0x00200000
#define		KU_CODE_ADJ					0x00400000
#define		KU_CODE_FIRST_ADJ			0x00800000
#define		KU_CODE_DO_POIREG			0x10000000
#define		KU_CODE_WBR_MASK			0x00003F00
#define		KU_CODE_WBR_SHIFT			8
#define		KU_CODE_KRM_MASK			0x00070000
#define		KU_CODE_KRM_SHIFT			16
#define		KU_CODE_LBR_MASK			0xFF000000
#define		KU_CODE_LBR_SHIFT			24
*/
typedef struct ACMSTROB_
{	DWORD dwSize;					// размер пакета в байтах (включая ACMHEADER)
	DWORD dwType;					// ACM_TYPE_STROB
	ACMHEADER s;
#ifndef DSP_BIOS
	ACMIMPULS acmImp[];
#endif	// DSP_BIOS
} ACMSTROB, * PACMSTROB;

typedef struct ACMSRAW_
{	DWORD dwSize;					// размер пакета в байтах (включая ACMHEADER)
	DWORD dwType;					// ACM_TYPE_RAW16 или ACM_TYPE_RAW1
	ACMHEADER s;
#ifndef DSP_BIOS
	USHORT usData[];
#endif	// DSP_BIOS
} ACMSRAW, * PACMSRAW;

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
