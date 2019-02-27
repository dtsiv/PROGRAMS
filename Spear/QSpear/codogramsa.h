#ifndef CODOGRAMS_H
#define CODOGRAMS_H

#define CODOGRAMS_H_VER_MAJOR 2
#define CODOGRAMS_H_VER_MINOR 3

// Qt types like quint32
#include <QtGlobal>

//=====================================================================
// input codograms
//=====================================================================
#define CGT_TEXT              0x4000
typedef struct {
	quint16 *sText[];
} CG_TEXT;

#define CGT_STATUS            0x4001
typedef struct {
	quint8 ud          : 8;        // indicator UD01
	quint8 pp4_1       : 8;        // indicator 4PP01-1
	quint8 pp4_2       : 8;        // indicator 4PP01-2
	quint8 pp4_3       : 8;        // indicator 4PP01-3
	quint8 pp4_4       : 8;        // indicator 4PP01-4
	quint8 pp3         : 8;        // indicator 3PP01
	quint8 cs1         : 8;        // indicator CS01-1
	quint8 cs2         : 8;        // indicator CS01-2
	quint8 gg          : 8;        // indicator GG01
	quint8 su          : 8;        // indicator SU01
	quint8 Tgr         : 8;        // indicator Temperature
	quint8 emi         : 8;        // indicator Irradiation
	quint8 rdy         : 8;        // indicator Readiness
	quint8 enT         : 8;        // timer enabled
} CG_STATUS;

#define CGT_ANT_SENSOR        0x4002
typedef struct {
	quint64  time      :64;
	quint32  flags     :32;
	float    azimuth   ;
	float    elevation ;
	quint32  padding   :32;
} CG_ANT_SENSOR;

//=====================================================================
// output codograms
//=====================================================================
#define CGT_CTRL              0x2000
typedef struct {
	union typeCtrlUnion {
		struct typeCtrlStruct {
			bool bIrr :1;				// allow irradiation
			bool bPow :1;				// turn power on
		} ctrlStruct1;
		quint16 ctrl;
	} ctrlUnion1;
} CG_CTRL;

#define CGT_MODE              0x2001
typedef struct {
	union typeModeUnion {
		struct typeModeStruct {
			bool bWork   :1;	        // work
			bool bFnCtrl :1;			// function control
		} modeStruct1;
		quint16 usMode;
	} modeUnion1;
} CG_MODE;

#define CGT_ANT_POS           0x2002
typedef struct {
	quint16 usBeta     : 16;
	quint16 usEpsilon  : 16;
} CG_ANT_POS;

#define CGT_ROTATE            0x2003
typedef struct {
	quint8 Speed      : 8;       // rotation speed 1....5
	quint8 Direct     : 8;       // direction: 1-up,2-down,3-right,4-left,5-stop
} CG_ROTATE;

#define CGT_INI               0x2004
typedef struct {
	float freq;                   // working frequency
	float AzCannon;               // azimuth of cannon
	float ElCannon;               // elevation of cannon
	float AzAnt;                  // azimuth of remote antenna
	float ElAnt;                  // elevation of remote antenna
} CG_INI;   

#endif // CODOGRAMS_H

