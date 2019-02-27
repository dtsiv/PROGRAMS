#ifndef FINDICATORLINE_H
#define FINDICATORLINE_H

#include <QtGui>
#include <QGLWidget>
#include "qfindicatorwidget.h"

#ifndef __linux
#include "../include/fftw3.h"
#include "codograms.h"
#else
#include <sys/time.h>
#include "fftw3.h"
#include "codograms_and_qavtctrl.h"
#endif

#define ENAB_VERTIC		0x10
#define ENAB_HORIZ		0x20
#define ENAB_SHOW_T		0x100
#define ENAB_SHOW_POIT	0x200
#define ENAB_SHOW_P0	0x400
#define ENAB_SHOW_P1	0x800
#define ENAB_SHOW_P2	0x1000
#define ENAB_SHOW_P3	0x2000
#define ENAB_SHOW_RAW	0x4000
#define ENAB_TA_REQUEST	0x8000
#define ENAB_SHOW_FFT	0x10000
class TFIndicatorLine;
class TFTrace;
class TCheckTread;
class TFFormular;
enum mmAction { mmNone, mmFormMove, mmHand, mmSelect };
//******************************************************************************
//
//******************************************************************************
class TFIndicatorLineStyle
{

public:
	TFIndicatorLineStyle();
	TFIndicatorLineStyle(TFIndicatorLine * pl);

public:
	double m_dExtentMinX, m_dExtentMinY;
	QPointF m_Extent, m_Ofset;
	QColor m_BkgColor;
	QColor m_clrLineVertic;
	QColor m_clrTextVertic;
	QColor m_clrLineHoriz;
	QColor m_clrTextHoriz;

	QColor m_clrFill;
	QColor m_clrOutline;
	QColor m_clrText;
	QColor m_clrTrace;
	QColor m_clrPoit;
	QColor m_clrP0;
	QColor m_clrP1;
	QColor m_clrP2;
	QColor m_clrP3;
	int m_PId0, m_PId1, m_PId2, m_PId3;
	double m_dFFrom;
	double m_dFTo;
	int m_iTTL;
	int m_iFormType;

	double m_dYMax;
	int m_EnabMask;
	int m_WebDencityX, m_WebDencityY;
	double m_dAzFrom;
	double m_dAzTo;
	int m_iComPoints;
	QFont m_WFont;
	QFont m_FFont;

};
//******************************************************************************
//
//******************************************************************************
class TFIndicatorLine : public QGLWidget
{
	Q_OBJECT

public:
	TFIndicatorLine(int id, QWidget * parent);
	TFIndicatorLine(int id, TFIndicatorLineStyle * pl, QWidget * parent);
	~TFIndicatorLine();
    void rebuild(void);
	void lock(void) { m_Lock.lock(); }
	void unlock(void) { m_Lock.unlock(); }
	QPointF extent() { return(m_s.m_Extent); }
	int id(void) { return(m_id); }
	void Logic2Client(QPointF * pfpo)
	{	pfpo -> rx() -= m_s.m_Ofset.x();
		pfpo -> ry() -= -m_s.m_Ofset.y();
		pfpo -> rx() *= m_s.m_Extent.x();
		pfpo -> ry() *= -m_s.m_Extent.y();
	}
	void Client2Logic(QPointF * pfpo)
	{	pfpo -> rx() /= m_s.m_Extent.x();
		pfpo -> ry() /= -m_s.m_Extent.y();
		pfpo -> rx() += m_s.m_Ofset.x();
		pfpo -> ry() += -m_s.m_Ofset.y();
	}
	void AddPoints(PTRACEINFOHEADER ptih);
	void AddPointsE(PTRACEINFOHEADER ptih);
	void AddPoints(PPOIT ppoit);
	void AddPoints(PPOITE ppoit);
	void AddPoints(PPOSTT ppt);
	void AddScope(PSCOPE16C pscope);
	void clear(void);
	void CheckFormularPointed(TFFormular * pf);
	void updateFrSel(PFRSELECTION ps)
	{	if(ps == NULL)
		{	m_dFrSelected = 0.;
			m_dBandSelected = 0.;
		} else
		{	m_dFrSelected = ps -> dFSel;
			m_dBandSelected = ps -> dBandSel;
			m_s.m_dAzFrom = ps -> dAzFrom;
			m_s.m_dAzTo = ps -> dAzTo;
			m_s.m_iComPoints = ps -> iCPCount;
			if(ps -> iFlags & FRSEL_FLAG_TA) m_s.m_EnabMask |= ENAB_TA_REQUEST;
			else m_s.m_EnabMask &= ~ENAB_TA_REQUEST;
		}
		update();
	}
	bool isAzimuthMatch(int iAz)
	{	double dAz = 0.001 * (double)iAz;
		return(dAz >= m_s.m_dAzFrom && dAz < m_s.m_dAzTo);
	}
					
private:
	QPointF m_MousePos;
	mmAction m_MouseAction;
	double m_dFrSelected, m_dBandSelected;
	int m_id;
	void draw(QPainter * pp);
	void drawRaw(QPainter * pp);
	void drawWeb(QPainter * pp);
	QMutex m_Lock;
	TCheckTread * m_pCheckTread;
	TFFormular * m_FormularPointed;
    fftw_plan m_fftPplan;
	fftw_complex * m_pFftIn, * m_pFftOut;
	double * m_dFftModul;
	double m_dFrCent, m_dFrInc;
	int m_iFftPId;

public slots:

signals:

public:
	TFIndicatorLineStyle m_s;
	QList<TFTrace *> m_FTraceList;
	TFTrace * m_FTracePoit;
	TFTrace * m_FTraceP0;
	TFTrace * m_FTraceP1;
	TFTrace * m_FTraceP2;
	TFTrace * m_FTraceP3;

protected:
	void resizeGL(int width, int height);
	void wheelEvent(QWheelEvent * event);
    void paintEvent(QPaintEvent * event);
//	void mouseDoubleClickEvent(QMouseEvent * event); 
	void mouseMoveEvent(QMouseEvent * event); 
	void mousePressEvent(QMouseEvent * event); 
	void mouseReleaseEvent(QMouseEvent * event); 
	void keyPressEvent(QKeyEvent * event);
};
//*****************************************************************************
//
//*****************************************************************************
class TCheckTread : public QThread
{
public:
	TCheckTread(TFIndicatorLine * pOwner) : QThread()
	{	m_pFIndicator = pOwner;
		m_bTermFlag = false;
		start();
	}
	~TCheckTread();
	bool ThreadStop()
	{	m_bTermFlag = true;
		return(wait(60000));
	}

public:

private:
	void run();
	TFIndicatorLine * m_pFIndicator;
	volatile bool m_bTermFlag;
};

#endif // FINDICATORLINE_H
