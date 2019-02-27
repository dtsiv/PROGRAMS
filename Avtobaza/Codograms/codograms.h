//******************************************************************************
//
//  ���� codograms.h - ��������� ���������� psadurs.exe - ������
//     ���������� �������� A, ������ ��� � ��� Copyright by Tristan 2009-2014
//     ��������� ���������� ������ � ���, ��� � ���
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
// ��� ������ �� Ps4 ������ ������ ����� ���������������� ����� dwType �
// ������������ � ��� ��� ����������. ����� ���������� ������������ ����
//
//******************************************************************************
#define  WTYP_ADUINFO			1       // ������ ����(������ �� ������������)
#define  WTYP_ADUINFOEX			2       // ����� ���� (������ ZONEINFO), -1 - ����� ����
#define  WTYP_VOIINFO			3       // ����� ������ ������� ����������� ������� � ���� (������ ZONEINFO)
#define  WTYP_POSTINFO			4       // ���������� ������ (����������)
#define  WTYP_GROUNDINFO		5       // ���������� �������� ������� (����������)
#define  WTYP_CTLINFO			6       // ���������� ��� ���������� (��������� LOCALCTRLSTRUCT)
#define  WTYP_FREQUCHANGE		7       // ���������� ��� ���������� ��������� (��������� FREQUTBL)
#define  WTYP_FDSTATUS			8		// ��� ������ (��������� FD_STATUS)
#define  WTYP_JUSTIFYRX			9       // ���������� ��������� (������ double[8])
#define  WTYP_CTLINFOEX			10      // ���������� ��� ���������� (��������� LOCALCTRLSTRUCTEX)

#define  WTYP_NEWCONNECTION		0xe     // � ������� ��������� ����� �����������
#define  WTYP_POSTADDR			0x10    // ������ ������ (��������� POSTADDRESSES)
#define  WTYP_CONNECTTOSERVER	0x11	// ������� ������� ������
#define  WTYP_POSTINFOE			0x12    // ���������� ������ (��������� POSTCORD)
#define  WTYP_GROUNDINFOE		0x13    // ���������� �������� ������� (������ GROUNDINFO)
#define  WTYP_CHANGEMODE		0x14    // ������� ������� ����� ������ (��������� APPLAYLOCALCTRL)
#define  WTYP_LNASTAT			0x15
#define  WTYP_CHANGEMODEEX		0x16    // ������� ������� ����� ������ (��������� APPLAYLOCALCTRLEX)

#define  WTYP_REGINFO       500         // ����� � ��� (FTABENT) ����� �� ������������
#define  WTYP_ET            1000        // ���������� (������ ETVOI) - ������
                                        // uFlags - ������� � ������ ������, ������ � �� �� ������ ������,
                                        // �� ��������� ������ �������� � ��/c
#define  WTYP_POIT          2000        // ���������� (������ POIT),
#define  WTYP_POITE         2001        // ���������� (������ POITE),
                                        // ��������� ���� �� ����
#define  WTYP_KT            2500        // ���
#define  WTYP_POSTT			2501		// ���������� (������ POSTT)	
#define  WTYP_POSTTE		2502		// ���������� (������ POSTTE)	
#define  WTYP_CONSOLE       2005        // �����
#define  WTYP_CONSOLEEX     2006        // ������������ ����� (��������� CONSOLEMSG)
#define  WTYP_TRACELIST     2009        // ������ �������� TRACELISTENTRY 
#define  WTYP_ADUCOM        3000        // ������� ��� ������ ����������
#define  WTYP_ADURSTATUS    3005        // ������ ������ ����������
#define	 WTYP_AVTSTATUS		0xb00		// ������ ������ ���������� (Avtobaza)
#define	 WTYP_UVSSTATUS		0xb01		// ������ ������ ���������� (Avtobaza)
#define  WTYP_VOICOM        4000        // RMOCOMMAND
#define  WTYP_VOIREP        4001        // RMOCOMMAND
#define  WTYP_FRSELECTION	4002        // FRSELECTION
#define  WTYP_TRACEINFO		4009		// TRACEINFOHEADER � TRACEINFOENTRY 
#define  WTYP_TRACEINFOE	4010		// TRACEINFOHEADER � TRACEINFOENTRYE (new style)
#define  WTYP_SCOPEDATA		6000        // �������������
#define  WTYP_SCOPE16C		6001        // ������������� (��������� SCOPE16C)
#define  WTYP_BASECORR		6010        // ���������� (BASECORRELATION)
#define  WTYP_TBOLTINFO     7100        // ���������� �� Trimble ThuderBolt
#define  WTYP_SOIMESSAGE	30006
#define  WTYPE_RAW1			0x8005		// ������������� 1 ���(��������� ACMHEADER)

//******************************************************************************
// ��������� �������� ���������� ��������� � ������������ ���� ������
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
// ��������� ZONEINFO
// � ����� ���� ��������� �������� � ���������� ����� �������� ���������
// ��������� ������� (���� ���������������, ������� ������� � ��.)
// ������ ��������� ��� ������������� (�������� 32-��� ��������) �
// ������������� ����������� (������� ���������� ������� � ��������)
//
//******************************************************************************
#ifndef ZONEINFO
typedef struct ZONEINFO_
{   int iID;                  // ����� ����/�������
    int iCount;               // ����� ������
    BLPOINT pt[32];            // ���������� � �������� (������, �������)
} ZONEINFO, * PZONEINFO;
#endif
//******************************************************************************
// ��������� GROUNDINFO
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
// ��������� POSTCORD (WTYP_POSTINFOE)
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
		DWORD dwTimeOut;	// [0] - timeout, [1] - FrGate, [2] - DurGate
		DWORD dwFrGate;
		DWORD dwDurGate;
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
//			LOCALCTRLSTRUCTEX
//******************************************************************************
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
		short sFrequHb;				// ������� ��������� WBR (2 - 18 ���) � 10 ���-��� ������� 
		short sFrequLb;				// ������� ��������� LBR (0.2 - 2 ���) � ���
		};
		DWORD dwFrequ;
	} freq[];
} FREQUTBL, * PFREQUTBL;
//******************************************************************************
// ��������� CONSOLEMSG
//******************************************************************************
typedef struct CONSOLEMSG_
{   union {
	FILETIME ftTime;				// ����� (100ns ticks)
    unsigned __int64 uqTime;
	};
	int iId;
	wchar_t sMessage[];
} CONSOLEMSG, * PCONSOLEMSG;

//******************************************************************************
// ��������� NEWCONNECTION (� ������� ��������� ����� �����������)
//******************************************************************************
typedef struct NEWCONNECTION_		// wtype WTYP_NEWCONNECTION
{   union {
	FILETIME ftTime;				// ����� UTC (100ns ticks)
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
// ��������� RMOCOMMAND
// ���������� ������� �� ���
//******************************************************************************
typedef struct RMOCOMMAND_  // dwType=4000
{   unsigned long Id;       // ��� ������� VVOD1, SBROSOPR, KORR
	unsigned long rm;       // ����� �������� �����
	unsigned long uNum;     // Trace Num
	unsigned long uFlags;   // Err code
	union {
		struct {
			double dLat;            // Lat ������� ��. ��� ������ ��
			double dLon;     	    // Lon �������
		};
		struct {
			double dTStart;         // ��������� ����� ���, 0. - ������ 
			double dTEnd;     	    // �������� ����� ���, -1. - �� �����
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
// ��������� FRSELECTION
// ���������� ������� �� ���
//******************************************************************************
typedef struct FRSELECTION_		// dwType=WTYP_FRSELECTION (4002)
{	unsigned long rm;			// ����� �������� �����
	unsigned long id;			// ����� ������� ��������� �������� 
	double dFSel;				// ����������� ������� ���
	double dBandSel;			// ���������� ���	
	double dAzFrom;				// ������ ��, ��. 
	double dAzTo;				// ������ ��, ��.
	int iCPCount;				// ����� ����� � ������������������ �����
	int iFlags;					//����� FRSEL_FLAG_XXX
	int iReserved[4];
} FRSELECTION, * PFRSELECTION;

#define    FRSEL_FLAG_TA		0x1
#define    FRSEL_FLAG_CLEARALL	0x2
//******************************************************************************
// ��������� ETVOI
// ���������� ����������������� ����� (��) - ����� ���
//******************************************************************************
typedef struct ETVOI_               // dwType = WTYP_ET (1000)
{   union {
	FILETIME ftTime;				// ����� ������������� (100ns ticks)
    unsigned __int64 uqTime;
	};
	unsigned long uTrNum;	        // ����� ������
    unsigned short usFlags;         // ����� ET_FLAGS_����
    unsigned short usStyle;         // �������������� �������� (����� �����������)
	union {
	double dAzimuth;	
	LBH lbh;						// ������� � ��	
	};
    XYH velo;                       // �������� � ��/c
	int iSmodeAdr;					// S-mode bort adress
	int iReserved;
	char bSmodeCall[8];				// S-mode call sign
	DWORD dwReserved[8];
} ETVOI, * PETVOI;

#define  ET_FLAGS_NEW           0x001     // ����� ������
#define  ET_FLAGS_AUTO          0x002     // �������������
#define  ET_FLAGS_CANCELED      0x004     // ����������
#define  ET_FLAGS_GROUND        0x008     // �������� �����
#define  ET_FLAGS_STATIC        0x010     // ����������� ������
#define  ET_FLAGS_IMIT	        0x020     // ������������� �����
#define  ET_FLAGS_AZIMUTH	    0x040     // ������ ��������
#define  ET_FLAGS_2D			0x100     // 2D �������
#define  ET_FLAGS_3D			0x200     // 3D �������
#define  ET_FLAGS_SMODADR		0x1000    // S-mode bort adress valid
#define  ET_FLAGS_SMODCS		0x2000    // S-mode call sign valid


//******************************************************************************
// ��������� POIT_EXINFO
//
//******************************************************************************
typedef union POIT_EXINFO_
{   struct {                    // RL,TACAN (������ ��������� ��������� ������������ �������)
	    unsigned Dummy0    :3;  // �����
	    unsigned Mod       :3;  // ��� ��������� (1-���,2-��-,4-��+)
	    unsigned Pol       :2;  // ��� �����������
	    unsigned Pim       :1;  // ������� ��������
	    unsigned PmodT     :1;  // ������� ��������� �
	    unsigned PmodTau   :1;  // ������� ��������� ���
        unsigned B		   :1;  /*��� ������� (��� tau > 400 ��� B = 1, ����� B = 0)*/
        unsigned H		   :1;  /*������� ��������� ���������*/
        unsigned ModF      :1;  /*������� ��������� �� �������� (������ ����)*/
	    unsigned NTacan    :18; //
		union {
		int Sector;
		int PostID;
		};
		float Aimp;             // ��������� �������� ��
	    float sigmataui;        // ��� ������������ �������� ���
	    float sigmaT;           // ��� ������� ���������� ��������� ���
	    float sigmafc;          // ��� ������� �������
    };

    struct {                    // SIF (������ ��������� ��������� ������������ �������)
	    unsigned long uDummyS;		// �����
	    int Sector;
	    float Aimp;             // ��������� �������� ��
        unsigned D4_C1	   :12; /*����������� ��� �������*/
        unsigned UBD	   :17; /*����������� ��� ������ ���-�*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (3)*/
   /*����� 1*/
        unsigned DP1	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (4)*/
   /*����� 2*/
        unsigned DP2	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (5)*/
   /*����� 3*/
        unsigned DP3	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (6)*/
   /*����� 4*/
	    unsigned long Rezhim;   // ������ SIF
    };

    struct {						// ������ �������� �������
	    unsigned short uDummyA;		// �����
	    unsigned short uFlags;		// �����
	    float fR, fBeta, fEps, fVdop;       // ���������, ������, ���� �����, ���������� ��������
	    float fDR, fDBeta, fDEps, fDVdop;   // ��������������� ����� ������ ���� �������
        float fAmp;
        BYTE bBeem;
        BYTE bSId;
        BYTE bPId;
        BYTE bCId;
        DWORD dwDopMask;
	    float DummyA[5];        // ������ ���������� ��������� ��� (��� ���������)
    };
} POIT_EXINFO, * PPOIT_EXINFO;
//******************************************************************************
// ��������� RXINFO
//
//******************************************************************************
typedef struct RXINFO_
{   unsigned short uMinuIndx, uSubtIndx;  // ��� ���������� ������ (������ � blh)
    int iIndex;             // ����� ���. � ����� �� 0
    unsigned long uT;       // uT - ��������� �������� �� ������ �� � ��
	union {
	double Az;				// ������ � ��������
	double D;				// �� � �
	};
	union {
	double sAz;				// ��� ������� � ���
	double sD;				// ��� �� � �
	};
    double dTau, dF;        // ���� ��� ���; ������� ���
} RXINFO, * PRXINFO;
//******************************************************************************
// ��������� POIT
//******************************************************************************
typedef struct POIT_
{   union {
	FILETIME ftTlock;				//����� �������(100ns ticks)
    unsigned __int64 uqTlock;
	};
    POIT_EXINFO exi;
	union {
	LBH lbh;
	BLH blh[8];                     // 0 - ViewPoint , ������� ������� ��
	};                              // 0 for KT - Main Point , ������� ������� ��
    unsigned long uFlags;           // POIT_FLAGS_CHNL_��� | POIT_FLAGS_����
	int Count;                      // ��� �������� ������� 0
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
// ��������� RXINFOE
//
//******************************************************************************
typedef struct RXINFOE_
{   unsigned short uMinuIndx, uSubtIndx;  // ��� ���������� ������ (������ � blh)
    int iIndex;             // ����� ���. � ����� �� 0
    unsigned long uT;       // uT - ��������� �������� �� ������ �� � ��
	union {
	double Az;				// ������ � ��������
	double D;				// �� � �
	};
	union {
	double sAz;				// ��� ������� � ���
	double sD;				// ��� �� � �
	};
	double dAmp;			// ��������� �� 
	double dTau;			// ���� ��� ���
	double dF;				// ����. ������� ���
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// ������ � ����� � ��
	USHORT usImpCount;		// ����� ���. � �����
	short sFreq0, sFreq1;	// KHz �� �����������
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
//******************************************************************************
// ��������� POITE
//******************************************************************************
typedef struct POITE_
{   union {
	FILETIME ftTlock;				//����� �������(100ns ticks)
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
	BLH blh[9];                     // 0 - ViewPoint , ������� ������� ��
	};                              // 0 for KT - Main Point , ������� ������� ��
    unsigned long uFlags;           // POIT_FLAGS_CHNL_��� | POIT_FLAGS_����
	int Count;                      // ��� �������� ������� 0
    RXINFOE rx[];
} POITE, * PPOITE;
//******************************************************************************
// ��������� IMPINFO
//
//******************************************************************************
#ifndef RCTRLH
typedef struct COMBOPOINTS_
{   unsigned int uT;       // uT - ��������� �������� �� ������ �� � ��
	struct {
		short sRe;
		short sIm;
	} point[];	
} COMBOPOINTS, * PCOMBOPOINTS;
#endif

typedef struct IMPINFO_
{   long uT;				// uT - ��������� �������� �� ������ �� � ��
	DWORD dwFlags;			// FLAG_IMPULS_XXXX
	int iAmp;				// 0 - 32000
	int iDur;				// 10ns ������
	int iFreq;				// KHz
	short sFreq0, sFreq1;	// KHz �� �����������
	DWORD dwFrSigma2;
	DWORD dwPeriod;			// ������ � ��
	int icount;				// ����� ��������� � �����
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
	USHORT usFlags;					// ����� CHAN_FLAG_XXXXX
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
{   long uT;				// uT - ��������� �������� �� ������ �� � ��
	int iDur;				// 10ns ������
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
		short sFreq0, sFreq1;	// KHz �� �����������
		DWORD dwFrSigma2;
		DWORD dwPeriod;			// ������ � ��
		USHORT usPackCount;		// ����� ��������� � �����
		USHORT usAtt;
		};
	};
	USHORT usAmp;				// 0 - 32000
	USHORT usTrashHold;
	int iPeleng;				// peleng estimation in 1 / 0x10000 parts of 2*PI
	BYTE bImpFlags;				// FLAG_IMPULS_XXXX
	BYTE bChannelNum;	
	BYTE bChannelFlags;			// ����� CHAN_FLAG_XXXXX
	BYTE bReserved;	
} IMPINFOE, * PIMPINFOE;

//******************************************************************************
// ��������� POSTT
//******************************************************************************
typedef struct POSTT_
{	union {
	FILETIME ftTlock;				//����� �������(100ns ticks)
	unsigned __int64 uqTlock;
	};
	DWORD dwDuration;               // ������������ (100ns ticks)
	int iPosId;
	DWORD dwFlags;                  // ����� FLAG_XXX_XXXX
	DWORD dwKuCode;					// ����� FLAG_KUCODE_XXXX
	DWORD dwKuAttCode;				
	DWORD dwAzCode0;
	DWORD dwAzCode1;
	DWORD dwFreq0;
	DWORD dwFreq1;
	DWORD dwChannelMask;
	USHORT usPostMask;				// ����������� � ��� ���� � �������������� ������ 
	USHORT usIndex;	
	int iImpCount;
	DWORD dwReserved[2];
	IMPINFO imp[];
} POSTT, * PPOSTT;

typedef struct POSTTE_
{	union {
	FILETIME ftTlock;				//����� �������(100ns ticks)
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
// ��������� TRACELISTENTRY
//******************************************************************************
typedef struct TRACELISTENTRY_
{   union {
	FILETIME ftStartTime;				//����� ��������(100ns ticks)
    unsigned __int64 uqStartTime;
	};
	double dLifeTime;					// in seconds from uqStartTime
    double dEpsylon;					// ��������
    int iId;
	int iType;
    int iIll;							// ���������
    int iCount, iPackCount, iRlCount;
	int iBaseCount[5];
	int iSmodeAdr;
	char bSmodeCall[8];
} TRACELISTENTRY, * PTRACELISTENTRY;

//******************************************************************************
//
//******************************************************************************
typedef struct RXENTRY_
{   unsigned long uT;       // uT - ��������� �������� �� ������ �� � ��
    unsigned long uBasesCode;
	float fTau, fF;         // ���� ��� ���; ������� ���
} RXENTRY, * PRXENTRY;
//******************************************************************************
//
//******************************************************************************
typedef struct TRACEINFOENTRY_
{   union {
	FILETIME ftLocTime;				//����� ������� (100ns ticks)(UTC)
    unsigned __int64 uqLocTime;
	};
	union {
	struct {                    // RL,TACAN (������ ��������� ��������� ������������ �������)
	    unsigned Dummy0    :3;  // �����
	    unsigned Mod       :3;  // ��� ��������� (1-���,2-��-,4-��+)
	    unsigned Pol       :2;  // ��� �����������
	    unsigned Pim       :1;  // ������� ��������
	    unsigned PmodT     :1;  // ������� ��������� �
	    unsigned PmodTau   :1;  // ������� ��������� ���
        unsigned B		   :1;  /*��� ������� (��� tau > 400 ��� B = 1, ����� B = 0)*/
        unsigned H		   :1;  /*������� ��������� ���������*/
        unsigned ModF      :1;  /*������� ��������� �� �������� (������ ����)*/
	    unsigned NTacan    :18; //
	    int Sector;
		float Aimp;             // ��������� �������� ��
	    float sigmataui;        // ��� ������������ �������� ���
	    float sigmaT;           // ��� ������� ���������� ��������� ���
	    float sigmafc;          // ��� ������� �������
		};
    struct {                    // SIF (������ ��������� ��������� ������������ �������)
	    unsigned long uSifFlags;   // ������ SIF
	    int Sector;
	    float Aimp;             // ��������� �������� ��
        unsigned D4_C1	   :12; /*����������� ��� �������*/
        unsigned UBD	   :17; /*����������� ��� ������ ���-�*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (3)*/
   /*����� 1*/
        unsigned DP1	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (4)*/
   /*����� 2*/
        unsigned DP2	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (5)*/
   /*����� 3*/
        unsigned DP3	   :29; /*����������� ��� ������ ���-S*/
        unsigned		   :3;  /*����� ����� � ���������� ������� (6)*/
   /*����� 4*/
		};
	};
    unsigned long uFlags;           // POIT_FLAGS_CHNL_��� | POIT_FLAGS_����
    int RxCount;                    // ��� �������� ������� 0
    RXENTRY rx[];
} TRACEINFOENTRY, * PTRACEINFOENTRY;
//******************************************************************************
//
//******************************************************************************
typedef struct RXENTRYE_
{   unsigned long uT;       // uT - ��������� �������� �� ������ �� � ��
    unsigned long uBasesCode;
	float fTau, fF;         // ���� ��� ���; ������� ���
	float fAmp;
	DWORD dwFrSigma2;
	DWORD dwStrobEQuad;
	DWORD dwPeriod;			// ������ � ����� � ��
	USHORT usImpCount;		// ����� ���. � �����
	short sFreq0, sFreq1;	// KHz �� �����������
	USHORT usStrobE;
} RXENTRYE, * PRXENTRYE;
//******************************************************************************
//
//******************************************************************************
typedef struct TRACEINFOENTRYE_
{   union {
	FILETIME ftLocTime;				//����� ������� (100ns ticks)(UTC)
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
	unsigned long uFlags;           // POIT_FLAGS_CHNL_��� | POIT_FLAGS_����
    int RxCount;                    // ��� �������� ������� 0
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
	/*TRACEINFOENTRY te[]; ��� TRACEINFOENTRYE te[];*/
} TRACEINFOHEADER, * PTRACEINFOHEADER;

#define TRINF_ERR_OK			0x0
#define TRINF_ERR_TRNOTFOUND	0x1
#define TRINF_ERR_NOINFO		0x2
//******************************************************************************
// ��������� SCOPE16C
//******************************************************************************
typedef struct SCOPE16C_
{   union {
	FILETIME ftTlock;				// ����� �������(100ns ticks)
    unsigned __int64 uqTlock;
	};
	int iPosId;
	int iChanId;
	int iCount;						// ����� ����������� �������� ( sizeof(sData) / 4)
	int iTimeInc;					// ������ ������������� ns
	int iFrequency;					// � ���
	int iReserved[6];
	short sData[];
} SCOPE16C, * PSCOPE16C;
//******************************************************************************
// ��������� BASECORRELATION (w-type WTYP_BASECORR)
//******************************************************************************
typedef struct BASECORRELATION_
{   union {
	FILETIME ftTlock;				// ����� �������(100ns ticks)
    unsigned __int64 uqTlock;
	};
	int iMinuId;
	int iSubtId;
	int iChanId;
	int iCount;						// ����� ��������
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
{	unsigned __int64 uqTime;		// ����� UTC (100ns ticks)
	double dSncBufStat;				// ����� ������� � ���
	double dCliBufStat;				// ����� ������� � ���
	double dUvsBufStat;				// ����� ������� � ���
	double dAduBufStat;				// ����� ������� � ���
	double dOut0BufStat;			// ����� � ���
	double dOut1BufStat;			// ����� � ���
	double dSpeedIn;				// ����� �� ����� ���� � ���
	double dSpeedOut;				// ����� �� �������� ���� � ���
	double dOutLoad;				// ������� ���������� ��������� ������
	double dReserved;				//
	DWORD dwFlags;                  // ����� UVS_STAT_FLAG_XXXXX
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
	unsigned __int64 uqLastUpdateTime;		// ����� UTC (100ns ticks)
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

typedef struct ACMHEADER_ {
	unsigned __int64 uqExecTime;	// ����� ���������� UTC (100ns ticks)
	DWORD dwDuration;               // ������������ (100ns ticks)
	USHORT usPostId;
	USHORT usIndx;
	DWORD dwFlags;                  // ����� FLAG_ACMSTROB_XXXX
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
	USHORT usFlags;					// ����� CHAN_FLAG_XXXXX
	USHORT usTrashHold;
	USHORT usPar0;
	USHORT usPar1;
	USHORT usPar2;
	USHORT usAtt;
	USHORT usPostMask;				// ����������� � ��� ���� � �������������� ������ 
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
//					��� STATUS
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
{	unsigned __int64 uqTime;		// ����� UTC (100ns ticks)
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
*	��������� ���
******************************************************************************/
/*! ����������� ����� �������� */
#define FDC_TYPE_READY		200		//!< �������� ����������
#define FDC_TYPE_FUNC		201		//!< ������� �������������� ��������
#define FDC_TYPE_DIAG		202		//!< �������������-��������������� ��������
#define FDC_TYPE_COMM		203		//!< �������� ����� �����
#define FDC_TYPE_NAV		204		//!< �������� ������������� �������

/*! ����������� ��������� */
#define FDC_SS_APM			1		//!< ���
#define FDC_SS_BUS			2		//!< ���
#define	FDC_SS_RDMT1		3		//!< ���� 1
#define	FDC_SS_RDMT2		4		//!< ���� 2
#define	FDC_SS_WBA			5		//!< ���

/*! ����������� ��������� */
#define FDC_DEV_UNKNOWN			0					//<! ���������� �� ����������

/*! ���������� ��� */
#define FDC_APM_HB_LNA(r, d)	(1 + r * 8 + d)		/*!< ��� ��������� 2 � 18 ���
													* r - ��������:
													* 0 - 2...4 ���
													* 1 - 4...8 ���
													* 2 - 8...12 ���
													* 3 - 12...18 ���
													* d - ����� �� 0 �� 7
													*/												
#define FDC_APM_HB_IN_SWITCH(r)	(33 + r)			/*!< ������� ����������
													* r - ��������:
													* 0 - 2...4 ���
													* 1 - 4...8 ���
													* 2 - 8...12 ���
													* 3 - 12...18 ���
													*/
#define FDC_APM_HB_WBR(n)		(37 + n)			//!< ��� 0, 1
#define FDC_APM_HB_WBR0			37					//!< ��� 0
#define FDC_APM_HB_WBR1			38					//!< ��� 1
#define FDC_APM_HB_OUT_SWITCH	39					//!< �������� ���������� �������� ������������
#define FDC_APM_HB_KU			40					//!< ��002 �������� ������������
#define FDC_APM_LB_KU			41					//!< ��002 ������� ������������
#define FDC_APM_LB_LNA(r, d)	(43 + r * 8 + d)	/*!< ��� ��������� 0,2...2 ���
													* r - ��������:
													* 0 - 1...2 ���
													* 1 - 0,5...1 ���
													* 2 - 0,2...0,5 ���
													* d - �����������:
													* ��� ������� ��������� 0...3
													* ��� ������� ���������� 0...7
													*/
#define FDC_APM_LB_SWITCH(n)	(64 + n)			/*!< ����������� ������� ������������
													* 0 - ���������� ��������� 0,5...1 ���
													* 1 - ���������� ��������� 1...2 ���
													* 2 - �������� ����������
													* 3 - ���������� ��������� 0,8...2 ���
													*/

/*! ���������� ��� */
#define FDC_BUS_MODEM(n)		(1 + n)				/*!< ������:
													* 0 - �����
													* 1 - �����������
													* 2 - ������
													*/
#define FDC_BUS_RUBIDIUM		4					//!< �������� �������
#define FDC_BUS_UVS				5					//!< ���
#define FDC_BUS_POWER(n)		(6 + n)				//!< �������� ������� 12� 0...1

/*! ���������� ���� */
#define FDC_RDMT_CPC			1					//!< CPC600
#define FDC_RDMT_ESW			2					//!< ���������� ESW
#define FDC_RDMT_SNC			3					//!< ���003
#define FDC_RDMT_UAP1			4					//!< ���001 - 1
#define FDC_RDMT_PRU1			5					//!< ���001
#define FDC_RDMT_UDS			6					//!< ���001
#define FDC_RDMT_PRU2			7					//!< ���001�
#define FDC_RDMT_UAP2			8					//!< ���001 - 2
#define FDC_RDMT_KSO			9					//!< ���001
#define FDC_RDMT_RPM			10					//!< ���002
#define FDC_RDMT_UAP3			11					//!< ���001 - 3
#define FDC_RDMT_URP			12					//!< ���001

/*! ���������� ��� */
#define FDC_WBA_LL				1					//!< 2��01
#define FDC_WBA_CP1				2					//!< 2��01 - 1
#define FDC_WBA_CP2				3					//!< 2��01 - 2
#define FDC_WBA_CP(n)			(2 + n)				//!< 2��01 - 0...1
#define FDC_WBA_UU				4					//!< 2��01
#define FDC_WBA_HOST			5					//!< host


//-----------------------------------------------------------------------------
//   ����� ����� *.h
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif

