#include "stdafx.h"
#include "ftrace.h"
#include "proppage.h"
#include "qfindicatorwidget.h"
#include "findicatorline.h"
#ifdef __linux
#include <cmath>
#define _hypot hypot
#endif

#ifdef __linux
using namespace std;
#endif

//******************************************************************************
//
//******************************************************************************
TFTrace::TFTrace(TFIndicatorLine * pOwner)
{
	m_pFIndicator = pOwner;
	m_iId = 0;
#ifndef __linux
        GetSystemTimeAsFileTime((FILETIME *)&m_uqLastCheckTime);
        m_uqLastLocTime = m_uqLastCheckTime - 300000000;
#else
        struct timeval tv;
        if (gettimeofday(&tv,NULL)) return;
        m_uqLastLocTime = unix_ticks_to_windows_ticks(tv.tv_usec) - 300000000;
#endif
}
//******************************************************************************
//
//******************************************************************************
TFTrace::TFTrace(int iId, TFIndicatorLine * pOwner)
{
	m_pFIndicator = pOwner;
	m_iId = iId;
#ifndef __linux
    GetSystemTimeAsFileTime((FILETIME *)&m_uqLastCheckTime);
    m_uqLastLocTime = m_uqLastCheckTime - 300000000;
#else
    struct timeval tv;
    if (gettimeofday(&tv,NULL)) return;
    m_uqLastLocTime = unix_ticks_to_windows_ticks(tv.tv_usec) - 300000000;
#endif
}
//******************************************************************************
//
//******************************************************************************
TFTrace::~TFTrace()
{
//	DbgPrint(L"TFTrace::DESTRUCTOR Id=%ld points=%ld formulars=%ld",
//					m_iId, m_PointList.count(), m_pFormularList.count());
	while(m_pFormularList.count())
	{	TFFormular * p = m_pFormularList.first();
		m_pFormularList.removeFirst();
		delete p;
	}
	m_PointList.clear();
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::draw(QPainter * pp)
{
	int i = 0;
	while(i < m_PointList.count())
	{	FTRACEPOINT tp = m_PointList.at(i++);
		pp -> drawLine(QPointF(tp.dF, 0.), QPointF(tp.dF, tp.dAimp));
	}
	i = 0;
	while(i < m_pFormularList.count())
	{	TFFormular * pf = m_pFormularList.at(i++);
		pf -> draw(pp);
	}
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::AddPoints(PTRACEINFOENTRY pte, int iCount)
{
	int i = 0;
	while(i < iCount)
	{	int RxCount = pte -> RxCount;
        if(pte->uqLocTime  >  m_uqLastLocTime)
        {	int j = 0;
			unsigned long uTPiv = 0;
			while(j < RxCount)
			{	PRXENTRY prx = &pte -> rx[j++];
				FTRACEPOINT tfp;
#ifndef __linux
				GetSystemTimeAsFileTime(&tfp.ftTime);
#else
                struct timeval tv;
                if (gettimeofday(&tv,NULL)) return;
                tfp.uqTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
				tfp.iFlag = pte -> uFlags & 0xf;
#ifndef __linux
                tfp.dAimp = (double)pte -> Aimp;
#else
                tfp.dAimp = (double)pte ->au2.as1.Aimp;
#endif
                tfp.dF = (double)prx -> fF;
				tfp.dD = (double)prx -> fTau;
				tfp.dP = (double)(prx -> uT - uTPiv) * 0.001;
				tfp.dBw = 0.;
				if(RxCount == 1) tfp.dP = 0.;
				uTPiv = prx -> uT;
				TFFormular * pf = getFormular(&tfp);
				pf -> m_tfp = tfp;
				m_PointList.append(tfp);
			}
		}
		i++;
        m_uqLastLocTime = pte -> uqLocTime;
		pte = (PTRACEINFOENTRY)(((char *)pte) + sizeof(TRACEINFOENTRY) + RxCount * sizeof(RXENTRY));
	}
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::AddPoints(PTRACEINFOENTRYE pte, int iCount)
{
	int i = 0;
	while(i < iCount)
	{	int RxCount = pte -> RxCount;
        if(pte->uqLocTime  >  m_uqLastLocTime)
		{	int j = 0;
			unsigned long uTPiv = 0;
			while(j < RxCount)
			{	PRXENTRYE prx = &pte -> rx[j++];
				FTRACEPOINT tfp;
#ifndef __linux
                GetSystemTimeAsFileTime(&tfp.ftTime);
#else
                struct timeval tv;
                if (gettimeofday(&tv,NULL)) return;
                tfp.uqTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
                tfp.iFlag = pte -> uFlags & 0xf;
				tfp.dAimp = (double)prx -> fAmp;
				tfp.dF = (double)prx -> fF;
				tfp.dD = (double)prx -> fTau;
				tfp.dBw = 0.;
				tfp.dP = (double)(prx -> uT - uTPiv) * 0.001;
				if(RxCount == 1) tfp.dP = 0.;
				uTPiv = prx -> uT;
				TFFormular * pf = getFormular(&tfp);
				pf -> m_tfp = tfp;
				m_PointList.append(tfp);
			}
		}
		i++;
		m_uqLastLocTime = pte -> uqLocTime;
		pte = (PTRACEINFOENTRYE)(((char *)pte) + sizeof(TRACEINFOENTRYE) + RxCount * sizeof(RXENTRYE));
	}
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::AddPoints(PPOIT ppoit)
{
	int i = 0;
	unsigned long uTPiv = 0;
	while(i < ppoit -> Count)
	{	PRXINFO prx = &ppoit -> rx[i++];
		FTRACEPOINT tfp;
#ifndef __linux
        GetSystemTimeAsFileTime(&tfp.ftTime);
#else
        struct timeval tv;
        if (gettimeofday(&tv,NULL)) return;
        tfp.uqTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
        tfp.iFlag = ppoit -> uFlags & 0xf;
#ifndef __linux
        tfp.dAimp = (double)ppoit -> exi.Aimp;
#else
        tfp.dAimp = (double)ppoit -> exi.as1.Aimp;
#endif
		tfp.dF = prx -> dF;
		tfp.dD = prx -> dTau;
		tfp.dP = (double)(prx -> uT - uTPiv) * 0.001;
		uTPiv = prx -> uT;
		m_PointList.append(tfp);
	}
    m_uqLastLocTime = ppoit -> uqTlock;
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::AddPoints(PPOITE ppoit)
{
	int i = 0;
	unsigned long uTPiv = 0;
	while(i < ppoit -> Count)
	{	PRXINFOE prx = &ppoit -> rx[i++];
		FTRACEPOINT tfp;
#ifndef __linux
        GetSystemTimeAsFileTime(&tfp.ftTime);
#else
        struct timeval tv;
        if (gettimeofday(&tv,NULL)) return;
        tfp.uqTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
        tfp.iFlag = ppoit -> uFlags & 0xf;
		tfp.dAimp = prx -> dAmp;
		tfp.dF = prx -> dF;
		tfp.dD = prx -> dTau;
		tfp.dP = (double)(prx -> uT - uTPiv) * 0.001;
		uTPiv = prx -> uT;
		m_PointList.append(tfp);
	}
    m_uqLastLocTime = ppoit -> uqTlock;
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::AddPoints(PPOSTT ppt)
{
	int i = 0;
	unsigned long uTPiv = 0;
	PIMPINFO pimp = &ppt -> imp[0];
	while(i++ < ppt -> iImpCount)
	{	if(m_pFIndicator -> isAzimuthMatch(pimp -> iPeleng))
		{	FTRACEPOINT tfp;
#ifndef __linux
            GetSystemTimeAsFileTime(&tfp.ftTime);
#else
            struct timeval tv;
            if (gettimeofday(&tv,NULL)) return;
            tfp.uqTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
            tfp.iFlag = pimp -> dwFlags & 0xf;
			double dTrash = 1.;
			if(pimp -> usTrashHold) dTrash = (double)pimp -> usTrashHold;
			if(pimp -> iAmp > 0) tfp.dAimp = 20. * log10((double)pimp -> iAmp / dTrash);
			else tfp.dAimp = 0.;
			tfp.dF = 0.001 * (double)pimp -> iFreq;
			tfp.dD = 0.01 * (double)pimp -> iDur;
			tfp.dP = (double)(pimp -> uT - uTPiv) * 0.001;
			uTPiv = pimp -> uT;
			m_PointList.append(tfp);
		}
		pimp = QFIndicatorWidget::nextImp(pimp);
	}
	m_uqLastLocTime = ppt -> uqTlock;
}
//******************************************************************************
//
//******************************************************************************
TFFormular * TFTrace::getFormular(PFTRACEPOINT ptfp)
{	
	TFFormular * pf = NULL;
	int i = 0;
	while(i < m_pFormularList.count())
	{	pf = m_pFormularList.at(i++);
		if(fabs(pf -> m_tfp.dF - ptfp -> dF) < 100.) return(pf);
	}
	pf = new TFFormular(this);
	m_pFormularList.append(pf);
	return(pf);
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::CheckPoints(void)
{
#ifndef __linux
    GetSystemTimeAsFileTime((FILETIME *)&m_uqLastCheckTime);
#else
    struct timeval tv;
    if (gettimeofday(&tv,NULL)) return;
    m_uqLastLocTime = unix_ticks_to_windows_ticks(tv.tv_usec);
#endif
	double dCheckTime = 1e-7 * (double)m_uqLastCheckTime;
	double dTTL = 0.001 * (double)m_pFIndicator -> m_s.m_iTTL;
	while(m_PointList.count())
	{	FTRACEPOINT tp = m_PointList.first();
		double dTime = 1e-7 * (double)tp.uqTime;
		dTime += dTTL;
		if(dTime - dCheckTime < 0.) m_PointList.removeFirst();
		else break;
	}
	CheckFormulars();
}
//******************************************************************************
//
//******************************************************************************
void TFTrace::CheckFormulars(void)
{
	int i = 0;
	while(i < m_pFormularList.count())
	{	TFFormular * pf = m_pFormularList.at(i);
		int j = 0;
		while(j < m_PointList.count())
		{	FTRACEPOINT tp = m_PointList.at(j);
			if(fabs(pf -> m_tfp.dF - tp.dF) < 100.) break;
			j++;
		}
		if(j >= m_PointList.count())
		{	m_pFormularList.removeAt(i);
			continue;
		}
		i++;
	}
}
//*****************************************************************************
//					+++ TRequestTread +++
//*****************************************************************************
TRequestTread::TRequestTread(QFIndicatorWidget * pOwner) : QThread()
{	m_pFIndicator = pOwner;
	m_bTermFlag = false;
	start();
}
//*****************************************************************************
//
//*****************************************************************************
TRequestTread::~TRequestTread()
{
	m_RequestList.clear();
	// DbgPrint(L"Q-FIndicator: TRequestTread DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
void TRequestTread::run()
{   unsigned __int64 uqt, uqTime;
	bool bUpdate;	

	while(!m_bTermFlag)
    {   lock();
		int i = 0;
		while(i < m_RequestList.count())
		{	TFREQUST req = m_RequestList.at(i);
			if(req.dTTo - req.dTFrom > 0.01)
			{	double dFrom = req.dTFrom;
				req.dTFrom = req.dTTo;
				m_RequestList.removeAt(i);
				m_RequestList.append(req);
				unlock();
				m_pFIndicator -> sendInfoReuqest(req.iId, dFrom, req.dTTo);
				break;
			}
			i++;
		}
		if(i >= m_RequestList.count())
		{	unlock();
            msleep(500);
        } else msleep(50);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void TRequestTread::AddRequest(PTRACELISTENTRY p, int iCou)
{	PTRACELISTENTRY ptle = p;
	int i = 0;
	lock();
	while(i < iCou)
	{	if(!ptle -> iIll)
		{	int j = 0;
			PTFREQUST preq = NULL;
			while(j < m_RequestList.count())
			{	PTFREQUST pr = &m_RequestList[j++];
				if(pr -> iId == ptle -> iId)
				{	preq = pr;
					break;	
				}
			}
			if(preq) preq -> dTTo = ptle -> dLifeTime;
			else
			{	TFREQUST req;
				req.iId = ptle -> iId;
				req.dTTo = ptle -> dLifeTime;
				req.dTFrom = req.dTTo - 15.;
				if(req.dTFrom < 0.) req.dTFrom = 0.;
				m_RequestList.append(req);
			}
		}
		i++;
		ptle++;
	}
	i = 0;
	while(i < m_RequestList.count())
	{	TFREQUST req = m_RequestList.at(i);
		ptle = p;
		int j = 0;
		while(j < iCou)
		{	if(req.iId == ptle -> iId) break;
			j++;
			ptle++;
		}
		if(j >= iCou)
		{	m_RequestList.removeAt(i);
			continue;
		}
		i++;
	}
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
TFFormular::TFFormular(TFTrace * pOwner)
{
	m_pFTrace = pOwner;
	m_FormularOffs = QPointF(30., -60.);
}
//*****************************************************************************
//
//*****************************************************************************
TFFormular::~TFFormular()
{
	TFIndicatorLine * ps = m_pFTrace -> m_pFIndicator;
	ps -> CheckFormularPointed(this);
//	DbgPrint(L"TFFormular: DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
void TFFormular::draw(QPainter * pp)
{	QPointF pt;

	TFIndicatorLine * ps = m_pFTrace -> m_pFIndicator;
	if(ps -> m_s.m_iFormType == FORMULAR_NON) return;

	QTransform tf = pp -> worldTransform();
	pp -> save();
	pp -> resetTransform();
	pp -> setPen(QPen(ps -> m_s.m_clrOutline));
	pp -> setBrush(QBrush(ps -> m_s.m_clrFill));
	pp -> setFont(ps -> m_s.m_FFont);
	pt = QPointF(m_tfp.dF, 0.); 
	pt = tf.map(pt);
	
	QRectF rcBoud(0., 0., ps -> width(), ps -> height()); 
	m_bHeadVisible = true;
	QString qsId;
	qsId.setNum(m_pFTrace -> id());
	qsId += " A";
	QRectF rc = pp -> fontMetrics().boundingRect(qsId); 
	double dH = rc.height();
	double dW = rc.width();
	QString qsD, qsP;
	if(ps -> m_s.m_iFormType != FORMULAR_IDONLY)
	{	qsD.setNum(m_tfp.dD, 'f', 1); 
		rc = pp -> fontMetrics().boundingRect(qsD); 
		dH += rc.height();
		dW = max(dW, rc.width());
		if(ps -> m_s.m_iFormType != FORMULAR_DONLY)
		{	qsP.setNum(m_tfp.dP, 'f', 1);
			rc = pp -> fontMetrics().boundingRect(qsP); 
			dH += rc.height();
			dW = max(dW, rc.width());
		}
	}
	m_FormuRect = QRectF(0., 0., dW + 6, dH);
	m_FormuRect.moveTo(m_FormularOffs + pt);
	if(!rcBoud.contains(pt))
	{	pp -> restore();
		m_bHeadVisible = false;
		return;
	}
	if(m_FormuRect.top() < 0.) m_FormuRect.translate(QPointF(0., -m_FormuRect.top()));
	if(m_FormuRect.left() < 0.) m_FormuRect.translate(QPointF(-m_FormuRect.left(), 0.));
	if(m_FormuRect.bottom() > rcBoud.height()) m_FormuRect.translate(QPointF(0., rcBoud.height() - m_FormuRect.bottom()));
	if(m_FormuRect.right() > rcBoud.width()) m_FormuRect.translate(QPointF(rcBoud.width() - m_FormuRect.right(), 0.));

	pp -> drawRect(m_FormuRect);
	
	QPointF pt0(m_FormuRect.left(), m_FormuRect.top()); 
	double d0 = _hypot(pt.x() - pt0.x(), pt.y() - pt0.y());
	QPointF pt1(m_FormuRect.right(), m_FormuRect.top()); 
	double d1 = _hypot(pt.x() - pt1.x(), pt.y() - pt1.y());
	QPointF pt2(m_FormuRect.right(), m_FormuRect.bottom()); 
	double d2 = _hypot(pt.x() - pt2.x(), pt.y() - pt2.y());
	QPointF pt3(m_FormuRect.left(), m_FormuRect.bottom()); 
	double d3 = _hypot(pt.x() - pt3.x(), pt.y() - pt3.y());
	if(d1 < d0)
	{	d0 = d1;
		pt0 = pt1;
	}
	if(d2 < d0)
	{	d0 = d2;
		pt0 = pt2;
	}
	if(d3 < d0)
	{	d0 = d3;
		pt0 = pt3;
	}
	pp -> drawLine(pt, pt0); 
	
	pp -> setPen(QPen(ps -> m_s.m_clrText));
	pp -> setBrush(QBrush(Qt::NoBrush));

	QPointF p(m_FormuRect.left(), m_FormuRect.top());
	p += QPointF(3., rc.height() - 3.);
	pp -> drawText(p, qsId);
	if(ps -> m_s.m_iFormType != FORMULAR_IDONLY)
	{	p += QPointF(0., rc.height());
		pp -> drawText(p, qsD);
		if(ps -> m_s.m_iFormType != FORMULAR_DONLY)
		{	p += QPointF(0., rc.height());
			pp -> drawText(p, qsP);
		}
	}
	pp -> restore();
}
