#ifndef QFTRACE_H
#define QFTRACE_H

#include <QtGui>
#include <QList>
#include <QThread>
#include <math.h>
#ifndef __linux
#include "codograms.h"
#else
#include <sys/time.h>
#include "codograms_and_qavtctrl.h"
#endif
class QFIndicatorWidget;
class TFIndicatorLine;
class TFTrace;

#define RAD_TO_DEG	57.29577951308232
#define DEG_TO_RAD	.0174532925199432958

#define WINDOWS_TICK 10000000LL
#define SEC_TO_UNIX_EPOCH 11644473600LL

#ifdef __linux
unsigned long long inline unix_ticks_to_windows_ticks(suseconds_t unix_ticks) {
    return (10LL*unix_ticks+SEC_TO_UNIX_EPOCH*WINDOWS_TICK);
}
#endif

typedef struct FTRACEPOINT_
{
#ifdef __linux
    unsigned __int64 uqTime;            // Время (100ns ticks)
#else
	union  {
        FILETIME ftTime;				// Время (100ns ticks)
        unsigned __int64 uqTime;
    };
#endif
	int iFlag;
	double dF;
	double dD;
	double dP;
	double dAimp;
	double dBw;
} FTRACEPOINT, * PFTRACEPOINT;

typedef struct TFREQUST_
{   int iId;
	double dTFrom, dTTo;
} TFREQUST, * PTFREQUST;

#define FORMULAR_NON	0
#define FORMULAR_IDONLY	1
#define FORMULAR_DONLY	2
//*****************************************************************************
//
//*****************************************************************************
class TFFormular
{

public:
	TFFormular(TFTrace * pOwner);
	~TFFormular();
	void draw(QPainter * pp);
	bool IsInFormular(QPointF pt) { return(m_FormuRect.contains(pt)); }
	void moveFormuar(QPointF pt) { m_FormularOffs -= pt; }

public:
	QPointF m_FormularOffs;
	QRectF m_FormuRect;
	TFTrace * m_pFTrace;
	FTRACEPOINT m_tfp;
	bool m_bHeadVisible;

private:
};
//*****************************************************************************
//
//*****************************************************************************
class TFTrace
{

public:
	TFTrace(TFIndicatorLine * pOwner);
	TFTrace(int iId, TFIndicatorLine * pOwner);
	~TFTrace();
	void AddPoints(PTRACEINFOENTRY ptie, int iCount);
	void AddPoints(PTRACEINFOENTRYE ptie, int iCount);
	void AddPoints(PPOIT ppoit);
	void AddPoints(PPOITE ppoit);
	void AddPoints(PPOSTT ppt);
	int id() { return(m_iId); }
	void CheckPoints(void);
	bool IsEmpty() { return(m_PointList.count() == 0); }
	void draw(QPainter * pp);

public:
	unsigned __int64 m_uqLastCheckTime;
	TFIndicatorLine * m_pFIndicator;
	QList<TFFormular *> m_pFormularList;

private:
	QList<FTRACEPOINT> m_PointList;
	int m_iId;
	TFFormular * getFormular(PFTRACEPOINT ptfp);
	void CheckFormulars(void);
    // number of 100-nanosecond intervals
    // since  Jan 1, 1601
    unsigned __int64 m_uqLastLocTime;
};
//*****************************************************************************
//
//*****************************************************************************
class TRequestTread : public QThread
{
public:
	TRequestTread(QFIndicatorWidget * pOwner);
	~TRequestTread();
	bool ThreadStop()
	{	m_bTermFlag = true;
		return(wait(60000));
	}
	void lock(void) { m_Lock.lock(); }
	void unlock(void) { m_Lock.unlock(); }
	void AddRequest(PTRACELISTENTRY ptle, int iCou);

public:

private:
	void run();
	QMutex m_Lock;
	QFIndicatorWidget * m_pFIndicator;
	volatile bool m_bTermFlag;
	QList<TFREQUST> m_RequestList;
};

#endif // QFTRACE_H
