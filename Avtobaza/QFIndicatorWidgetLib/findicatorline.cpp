#include "stdafx.h"
#include "findicatorline.h"
#include "ftrace.h"

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
TFIndicatorLineStyle::TFIndicatorLineStyle()
{
    m_dExtentMinX = 1.0e0;
    m_dExtentMinY = 1.0e0;

	m_BkgColor = Qt::darkYellow;
	m_Extent = QPointF(1., -1.);
	m_Ofset = QPointF(0., 0.);
	m_EnabMask = ENAB_VERTIC | ENAB_HORIZ | ENAB_SHOW_T | ENAB_SHOW_RAW;
	m_clrLineVertic = QColor(0, 0, 0x80, 0x60);
	m_clrTextVertic = QColor(0, 0, 0x80, 0xff);
	m_clrLineHoriz = QColor(0, 0x40, 0x40, 0x60);
	m_clrTextHoriz = QColor(0, 0x40, 0x40, 0xff);;
	m_dYMax = 100.;
	m_WebDencityX = 7;
	m_WebDencityY = 3;
	m_clrFill = QColor(0x80, 0x80, 0x80, 0x80);
	m_clrOutline = QColor(0x80, 0x80, 0x80, 0x80);
	m_clrText = QColor(0x0, 0x0, 0x0, 0xff);
	m_clrTrace = QColor(0xff, 0x0, 0x0, 0x80);
	m_clrPoit = QColor(0x0, 0xff, 0x0, 0x80);
	m_clrP0 = QColor(0x0, 0xff, 0x0, 0x80);
	m_clrP1 = QColor(0x0, 0xff, 0x0, 0x80);
	m_clrP2 = QColor(0x0, 0xff, 0x0, 0x80);
	m_clrP3 = QColor(0x0, 0xff, 0x0, 0x80);
	m_PId0 = 1000;
	m_PId1 = 1001;
	m_PId2 = 1002;
	m_PId3 = 1003;
	m_dFFrom = 500.;
	m_dFTo = 10000.;
	m_iTTL = 10000;
	m_iFormType = 0;
	m_dAzFrom = 0.;
	m_dAzTo = 360.;
	m_iComPoints = 20;
}
//******************************************************************************
//
//******************************************************************************
TFIndicatorLineStyle::TFIndicatorLineStyle(TFIndicatorLine * pl)
{
	m_dExtentMinX = pl -> m_s.m_dExtentMinX;
	m_dExtentMinY = pl -> m_s.m_dExtentMinY;
	m_Extent = pl -> m_s.m_Extent; 
	m_Ofset = pl -> m_s.m_Ofset;
	m_BkgColor = pl -> m_s.m_BkgColor;
	m_clrLineVertic = pl -> m_s.m_clrLineVertic;
	m_clrTextVertic = pl -> m_s.m_clrTextVertic;
	m_clrLineHoriz = pl -> m_s.m_clrLineHoriz;
	m_clrTextHoriz = pl -> m_s.m_clrTextHoriz;
	m_dYMax = pl -> m_s.m_dYMax;
	m_EnabMask = pl -> m_s.m_EnabMask;
	m_WebDencityX = pl -> m_s.m_WebDencityX;
	m_WebDencityY = pl -> m_s.m_WebDencityY;
	m_WFont = pl -> m_s.m_WFont;
	m_FFont = pl -> m_s.m_FFont;
	m_clrFill = pl -> m_s.m_clrFill;
	m_clrOutline = pl -> m_s.m_clrOutline;
	m_clrText = pl -> m_s.m_clrText;
	m_clrTrace = pl -> m_s.m_clrTrace;
	m_clrPoit = pl -> m_s.m_clrPoit;
	m_clrP0 = pl -> m_s.m_clrP0;
	m_clrP1 = pl -> m_s.m_clrP1;
	m_clrP2 = pl -> m_s.m_clrP2;
	m_clrP3 = pl -> m_s.m_clrP3;
	m_PId0 = pl -> m_s.m_PId0;
	m_PId1 = pl -> m_s.m_PId1;
	m_PId2 = pl -> m_s.m_PId2;
	m_PId3 = pl -> m_s.m_PId3;
	m_dFFrom = pl -> m_s.m_dFFrom;
	m_dFTo = pl -> m_s.m_dFTo;
	m_iTTL = pl -> m_s.m_iTTL;
	m_iFormType = pl -> m_s.m_iFormType;
	m_dAzFrom = pl -> m_s.m_dAzFrom;
	m_dAzTo = pl -> m_s.m_dAzTo;
	m_iComPoints = pl -> m_s.m_iComPoints;

};
//******************************************************************************
//
//******************************************************************************
TFIndicatorLine::TFIndicatorLine(int id, QWidget * parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_id = id;
	m_FTracePoit = new TFTrace(99990, this);
	m_FTraceP0 = new TFTrace(99991, this);
	m_FTraceP1 = new TFTrace(99992, this);
	m_FTraceP2 = new TFTrace(99993, this);
	m_FTraceP3 = new TFTrace(99994, this);
	m_pCheckTread = new TCheckTread(this);
	m_dFrSelected = 0.;
	m_dBandSelected = 0.;
	m_dFrCent = 0.;
	m_dFrInc = 1.;
	m_iFftPId = 1;
	setFocusPolicy(Qt::ClickFocus);
	m_pFftIn = (fftw_complex *)fftw_malloc(4096 * sizeof(fftw_complex));
	m_pFftOut = (fftw_complex *)fftw_malloc(4096 * sizeof(fftw_complex));
	m_fftPplan = fftw_plan_dft_1d(4096, m_pFftIn, m_pFftOut, FFTW_FORWARD, FFTW_ESTIMATE);
	m_dFftModul = new double[4096];

}
//******************************************************************************
//
//******************************************************************************
TFIndicatorLine::TFIndicatorLine(int id, TFIndicatorLineStyle * pl, QWidget * parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_id = id;
	m_s = *pl;
	m_FTracePoit = new TFTrace(99990, this);
	m_FTraceP0 = new TFTrace(99991, this);
	m_FTraceP1 = new TFTrace(99992, this);
	m_FTraceP2 = new TFTrace(99993, this);
	m_FTraceP3 = new TFTrace(99994, this);
	m_pCheckTread = new TCheckTread(this);
	m_dFrSelected = 0.;
	m_dBandSelected = 0.;
	m_dFrCent = 0.;
	m_dFrInc = 1.;
	m_iFftPId = 1;
	setFocusPolicy(Qt::ClickFocus);
	m_pFftIn = (fftw_complex *)fftw_malloc(4096 * sizeof(fftw_complex));
	m_pFftOut = (fftw_complex *)fftw_malloc(4096 * sizeof(fftw_complex));
	m_fftPplan = fftw_plan_dft_1d(4096, m_pFftIn, m_pFftOut, FFTW_FORWARD, FFTW_ESTIMATE);
	m_dFftModul = new double[4096];
}
//******************************************************************************
//
//******************************************************************************
TFIndicatorLine::~TFIndicatorLine()
{
	m_pCheckTread -> ThreadStop();
	delete m_pCheckTread;
	// DbgPrint(L"TFIndicatorLine::DESTRUCTOR FtraceCount=%ld", m_FTraceList.count());
	while(m_FTraceList.count())
	{	TFTrace * p = m_FTraceList.first();
		m_FTraceList.removeFirst();
		delete p;
	}
	delete m_FTracePoit;
	delete m_FTraceP0;
	delete m_FTraceP1;
	delete m_FTraceP2;
	delete m_FTraceP3;
    fftw_destroy_plan(m_fftPplan);
    fftw_free(m_pFftIn);
    fftw_free(m_pFftOut);
	delete[] m_dFftModul;

}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::clear(void)
{
	lock();
	while(m_FTraceList.count())
	{	TFTrace * p = m_FTraceList.first();
		m_FTraceList.removeFirst();
		delete p;
	}
	unlock();
	update();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::resizeGL(int width, int height)
{
	QWheelEvent we(QPoint(width / 2, height / 2), 0, Qt::NoButton, Qt::ShiftModifier);
	m_s.m_dExtentMinX = (double)width / (m_s.m_dFTo - m_s.m_dFFrom);
    if (height==0) height=1;
    m_s.m_dExtentMinY = (double)height / m_s.m_dYMax;
	m_s.m_Extent.setY(-1.);
	wheelEvent(&we);
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine:: rebuild(void) {
    resizeGL(width(), height());
    return;
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::keyPressEvent(QKeyEvent * event)
{
	int iDelta = 0;
	if(event -> key() == Qt::Key_Equal) iDelta = 30;
	else if(event -> key() == Qt::Key_Minus) iDelta = -30;
	else return;

	QPointF fpo = m_MousePos;
    Logic2Client(&fpo);
	QWheelEvent we(QPoint((int)fpo.x(), (int)fpo.y()), iDelta, Qt::NoButton, event -> modifiers());
	wheelEvent(&we);
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::wheelEvent(QWheelEvent * event)
{	double f, dExt;

	if(event -> buttons() & Qt::MidButton || event -> modifiers() & Qt::AltModifier || event -> modifiers() & Qt::ShiftModifier)
	{	dExt = m_s.m_Extent.y();
		f = 1. / dExt;
		dExt *= pow(1.001, ((double)event -> delta()) / 1.);
		dExt = max(min(dExt, 60. * m_s.m_dExtentMinY), m_s.m_dExtentMinY);
		m_s.m_Extent.setY(dExt);
		f -= 1. / dExt;
		double dY = m_s.m_Ofset.y() + f * (double)event -> y();
		m_s.m_Ofset.setY(min(max(dY, -m_s.m_dYMax), -height() / dExt));
	} 
	if(!(event -> buttons() & Qt::MidButton || event -> modifiers() & Qt::AltModifier) || event -> modifiers() & Qt::ShiftModifier)
	{	dExt = m_s.m_Extent.x();
		f = 1. / dExt;
		dExt *= pow(1.001, ((double)event -> delta()) / 1.);
		dExt = max(min(dExt, 60. * m_s.m_dExtentMinX), m_s.m_dExtentMinX);
		m_s.m_Extent.setX(dExt);
		f -= 1. / dExt;
		double dX = m_s.m_Ofset.x() + f * (double)event -> x();
		m_s.m_Ofset.setX(max(min(dX, m_s.m_dFTo - width() / dExt), m_s.m_dFFrom));
	}
	update(); 
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::paintEvent(QPaintEvent * event)
{	QPainter painter;

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
	
	QBrush br(m_s.m_BkgColor);
    painter.fillRect(event -> rect(), br);
	if(m_dBandSelected > 0.000001)
	{	QColor qc = m_s.m_clrLineVertic;
		qc.setAlpha(128);
		br.setColor(qc);
		br.setStyle(Qt::Dense6Pattern);
		double dLeft = m_dFrSelected - m_dBandSelected;
		dLeft -= m_s.m_Ofset.x();
		dLeft *= m_s.m_Extent.x();
		double dWid = 2. * m_dBandSelected;
		dWid *= m_s.m_Extent.x();
		QRectF qr(dLeft, 0., dWid, (double)event -> rect().height());
		painter.fillRect(qr, br);
	}
	painter.scale(m_s.m_Extent.x(), -m_s.m_Extent.y());
	painter.translate(-m_s.m_Ofset.x(), m_s.m_Ofset.y());
	drawWeb(&painter);
	if(m_s.m_EnabMask & ENAB_SHOW_FFT) drawRaw(&painter);
	else draw(&painter);
	painter.end();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::draw(QPainter * pp)
{
	int i;
	QPen penOld = pp -> pen();
	lock();
	
	if(m_s.m_EnabMask & ENAB_SHOW_T)
	{	pp -> setPen(QColor(m_s.m_clrTrace));
		i = 0;
		while(i < m_FTraceList.count())
		{	TFTrace * pt = m_FTraceList.at(i++);
			pt -> draw(pp);
		}
	}
	if(m_s.m_EnabMask & ENAB_SHOW_POIT)
	{	pp -> setPen(QColor(m_s.m_clrPoit));
		m_FTracePoit -> draw(pp);
	}
	if(m_s.m_EnabMask & ENAB_SHOW_P0)
	{	pp -> setPen(QColor(m_s.m_clrP0));
		m_FTraceP0 -> draw(pp);
	}
	if(m_s.m_EnabMask & ENAB_SHOW_P1)
	{	pp -> setPen(QColor(m_s.m_clrP1));
		m_FTraceP1 -> draw(pp);
	}
	if(m_s.m_EnabMask & ENAB_SHOW_P2)
	{	pp -> setPen(QColor(m_s.m_clrP2));
		m_FTraceP2 -> draw(pp);
	}
	if(m_s.m_EnabMask & ENAB_SHOW_P3)
	{	pp -> setPen(QColor(m_s.m_clrP3));
		m_FTraceP3 -> draw(pp);
	}
	unlock();
	pp -> setPen(penOld);
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::drawRaw(QPainter * pp)
{	int i;
	QPen penOld = pp -> pen();
	lock();
	
	if(m_s.m_EnabMask & ENAB_SHOW_P0) pp -> setPen(QColor(m_s.m_clrP0));
	else if(m_s.m_EnabMask & ENAB_SHOW_P1) pp -> setPen(QColor(m_s.m_clrP1));
	else if(m_s.m_EnabMask & ENAB_SHOW_P2) pp -> setPen(QColor(m_s.m_clrP2));
	else if(m_s.m_EnabMask & ENAB_SHOW_P3) pp -> setPen(QColor(m_s.m_clrP3));

	i = 0;
	double dx0 = m_dFrCent - 2048. * m_dFrInc;
	double d0 = m_dFftModul[0];
	while(i++ < 4095)
	{	pp -> drawLine(QPointF(dx0, d0), QPointF(dx0 + m_dFrInc, m_dFftModul[i]));
		dx0 += m_dFrInc;
		d0 = m_dFftModul[i];
	}
	unlock();
	pp -> setPen(penOld);
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::drawWeb(QPainter * pp)
{	int i;
    double dStepT  = 1.0e0;
    double dStartT = 1.0e0;
    double dStepV  = 1.0e0;
    double dStartV = 1.0e0;
	QPointF p0(0., (double)height()), p1((double)width(), 0.);
	Client2Logic(&p0);
	Client2Logic(&p1);

	QFont fontOld = pp -> font();
	pp -> setFont(m_s.m_WFont);
	QPen penOld = pp -> pen();
	if(m_s.m_EnabMask & ENAB_VERTIC)
	{	pp -> setPen(m_s.m_clrLineVertic);
	
		double dT = p1.x() - p0.x();
		dStepT = 10000.;
		while(true)
		{	if(dT / dStepT >= (double)m_s.m_WebDencityX) break;
			dStepT /= 2.;
			if(dT / dStepT >= (double)m_s.m_WebDencityX) break;
			dStepT /= 2.5;
			if(dT / dStepT >= (double)m_s.m_WebDencityX) break;
			dStepT /= 2.;
		}
		i = p0.x() / dStepT + 1;	
		dStartT = dStepT * i;
	
		double dX = dStartT;
		while(dX < p1.x())
		{	pp -> drawLine(QPointF(dX, p0.y()), QPointF(dX, p1.y()));
			dX += dStepT;
		}
	}
	if(m_s.m_EnabMask & ENAB_HORIZ)
	{	pp -> setPen(m_s.m_clrLineHoriz);
	
		double dV = p1.y() - p0.y();
		dStepV = 10000.;
		while(true)
		{	if(dV / dStepV >= (double)m_s.m_WebDencityY) break;
			dStepV /= 2.;
			if(dV / dStepV >= (double)m_s.m_WebDencityY) break;
			dStepV /= 2.5;
			if(dV / dStepV >= (double)m_s.m_WebDencityY) break;
			dStepV /= 2.;
		}
		i = p0.y() / dStepV + 1;	
		dStartV = dStepV * i;
	
		double dY = dStartV;
		while(dY < p1.y())
		{	pp -> drawLine(QPointF(p0.x(), dY), QPointF(p1.x(), dY));
			dY += dStepV;
		}
	}

	QString qs, qsT, qsA;
	QPointF qp;
	QRectF br, brT, brA;
	QTransform tf = pp -> worldTransform();
	pp -> save();
	pp -> resetTransform();

	pp -> setPen(m_s.m_clrTextVertic);
	qsT = "freq mhz";
	brT = pp -> fontMetrics().boundingRect(qsT);

	if(m_s.m_EnabMask & ENAB_VERTIC)
	{	double dX = dStartT;
		double d = 2.5 * brT.width() / m_s.m_Extent.x(); 
		while(dX + d < p1.x())
		{	qs.setNum(dX, 'f', 1);	
			qp = tf.map(QPointF(dX, p0.y()));
			qp -= QPoint(-3., 3.);
			pp -> drawText(qp, qs);
			dX += dStepT;
		}
	}
	qp = tf.map(QPointF(p1.x(), p0.y()));
	qp -= QPoint(5. + brT.width(), 3.);
	pp -> drawText(qp, qsT);

	pp -> setPen(m_s.m_clrTextHoriz);
	qsT = "Amp db";
	brT = pp -> fontMetrics().boundingRect(qsT);

	if(m_s.m_EnabMask & ENAB_HORIZ)
	{	double dY = dStartV;
		double d = brT.height() / m_s.m_Extent.y(); 
		int iPes = 1;
		
		if(dStepV < 1.) iPes = 2;
		else if(dStepV < 10.) iPes = 1;
		else iPes = 0;

		while(dY + d < p1.y())
		{	qs.setNum(dY, 'f', iPes);	
			qp = tf.map(QPointF(p0.x(), dY));
			br = pp -> fontMetrics().boundingRect(qs);
			qp += QPoint(3., br.height() - 3.);
			pp -> drawText(qp, qs);
			dY += dStepV;
		}
	}
	qp = tf.map(QPointF(p0.x(), p1.y()));
	qp += QPoint(5., brT.height());
	pp -> drawText(qp, qsT);
	pp -> restore();
	pp -> setPen(penOld);
	pp -> setFont(fontOld);
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddPoints(PTRACEINFOHEADER ptih)
{
	lock();
	if(ptih -> bErrCode != TRINF_ERR_OK)
	{	unlock();
		return;
	}
	TFTrace * pft = NULL;
	int i = 0;
	while(i < m_FTraceList.count())
	{	TFTrace * p = m_FTraceList.at(i++);
		if(p -> id() == (int)ptih -> usId)
		{	pft = p;
			break;	
		}
	}
	if(pft == NULL) 
	{	pft = new TFTrace((int)ptih -> usId, this);
		m_FTraceList.append(pft);
	}
	PTRACEINFOENTRY pte = (PTRACEINFOENTRY)(ptih + 1);
	pft -> AddPoints(pte, ptih -> iTeCount);
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddPointsE(PTRACEINFOHEADER ptih)
{
	lock();
	if(ptih -> bErrCode != TRINF_ERR_OK)
	{	unlock();
		return;
	}
	TFTrace * pft = NULL;
	int i = 0;
	while(i < m_FTraceList.count())
	{	TFTrace * p = m_FTraceList.at(i++);
		if(p -> id() == (int)ptih -> usId)
		{	pft = p;
			break;	
		}
	}
	if(pft == NULL) 
	{	pft = new TFTrace((int)ptih -> usId, this);
		m_FTraceList.append(pft);
	}
	PTRACEINFOENTRYE pte = (PTRACEINFOENTRYE)(ptih + 1);
	pft -> AddPoints(pte, ptih -> iTeCount);
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddPoints(PPOIT ppoit)
{
	lock();
#ifndef __linux
	if(ppoit -> uFlags & POIT_FLAGS_POSTID_VALID)
	{	if(ppoit -> exi.PostID == m_s.m_PId0 && m_s.m_EnabMask & ENAB_SHOW_P0)
			m_FTraceP0 -> AddPoints(ppoit);
		if(ppoit -> exi.PostID == m_s.m_PId1 && m_s.m_EnabMask & ENAB_SHOW_P1)
			m_FTraceP1 -> AddPoints(ppoit);
		if(ppoit -> exi.PostID == m_s.m_PId2 && m_s.m_EnabMask & ENAB_SHOW_P2)
			m_FTraceP2 -> AddPoints(ppoit);
		if(ppoit -> exi.PostID == m_s.m_PId3 && m_s.m_EnabMask & ENAB_SHOW_P3)
			m_FTraceP3 -> AddPoints(ppoit);

	} else if(m_s.m_EnabMask & ENAB_SHOW_POIT) m_FTracePoit -> AddPoints(ppoit); 
#else
    if(ppoit -> uFlags & POIT_FLAGS_POSTID_VALID)
    {	if(ppoit -> exi.as1.au1.PostID == m_s.m_PId0 && m_s.m_EnabMask & ENAB_SHOW_P0)
            m_FTraceP0 -> AddPoints(ppoit);
        if(ppoit -> exi.as1.au1.PostID == m_s.m_PId1 && m_s.m_EnabMask & ENAB_SHOW_P1)
            m_FTraceP1 -> AddPoints(ppoit);
        if(ppoit -> exi.as1.au1.PostID == m_s.m_PId2 && m_s.m_EnabMask & ENAB_SHOW_P2)
            m_FTraceP2 -> AddPoints(ppoit);
        if(ppoit -> exi.as1.au1.PostID == m_s.m_PId3 && m_s.m_EnabMask & ENAB_SHOW_P3)
            m_FTraceP3 -> AddPoints(ppoit);

    } else if(m_s.m_EnabMask & ENAB_SHOW_POIT) m_FTracePoit -> AddPoints(ppoit);
#endif
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddPoints(PPOITE ppoit)
{
	lock();
	if(ppoit -> uFlags & POIT_FLAGS_POSTID_VALID)
	{	if(!(m_s.m_EnabMask & ENAB_SHOW_RAW))
		{	if(ppoit -> iPostID == m_s.m_PId0 && m_s.m_EnabMask & ENAB_SHOW_P0)
				m_FTraceP0 -> AddPoints(ppoit);
			if(ppoit -> iPostID == m_s.m_PId1 && m_s.m_EnabMask & ENAB_SHOW_P1)
				m_FTraceP1 -> AddPoints(ppoit);
			if(ppoit -> iPostID == m_s.m_PId2 && m_s.m_EnabMask & ENAB_SHOW_P2)
				m_FTraceP2 -> AddPoints(ppoit);
			if(ppoit -> iPostID == m_s.m_PId3 && m_s.m_EnabMask & ENAB_SHOW_P3)
				m_FTraceP3 -> AddPoints(ppoit);
		}
	} else if(m_s.m_EnabMask & ENAB_SHOW_POIT) m_FTracePoit -> AddPoints(ppoit); 
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddPoints(PPOSTT ppt)
{
	if(!(m_s.m_EnabMask & ENAB_SHOW_RAW)) return;
	lock();
	if(ppt -> iPosId == m_s.m_PId0 && m_s.m_EnabMask & ENAB_SHOW_P0)
		m_FTraceP0 -> AddPoints(ppt);
	if(ppt -> iPosId == m_s.m_PId1 && m_s.m_EnabMask & ENAB_SHOW_P1)
		m_FTraceP1 -> AddPoints(ppt);
	if(ppt -> iPosId == m_s.m_PId2 && m_s.m_EnabMask & ENAB_SHOW_P2)
		m_FTraceP2 -> AddPoints(ppt);
	if(ppt -> iPosId == m_s.m_PId3 && m_s.m_EnabMask & ENAB_SHOW_P3)
		m_FTraceP3 -> AddPoints(ppt);
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::AddScope(PSCOPE16C pscope)
{
	if(!(m_s.m_EnabMask & ENAB_SHOW_FFT)) return;
	if(!((pscope -> iPosId == m_s.m_PId0 && m_s.m_EnabMask & ENAB_SHOW_P0) ||
		(pscope -> iPosId == m_s.m_PId1 && m_s.m_EnabMask & ENAB_SHOW_P1) ||
		(pscope -> iPosId == m_s.m_PId2 && m_s.m_EnabMask & ENAB_SHOW_P2) ||
		(pscope -> iPosId == m_s.m_PId3 && m_s.m_EnabMask & ENAB_SHOW_P3))) return;
	lock();
	short * ps = &pscope -> sData[0];
	fftw_complex * pc = m_pFftIn;
	int i = 0;
	while(i < 4096 && i < pscope -> iCount)
	{	(*pc)[0] = (double)*ps++;
		(*pc)[1] = (double)*ps++;
		pc++;
		i++;
	}
	while(i < 4096)
	{	(*pc)[0] = 0.;
		(*pc)[1] = 0.;
		pc++;
		i++;
	}

	fftw_execute(m_fftPplan);

	pc = &m_pFftOut[2048];
	double * pd = m_dFftModul;
	i = 0;
	while(i < 2048)
	{	*pd = _hypot((*pc)[0], (*pc)[1]);
		pc++;
		if(*pd < 1.) *pd = 1.;
		*pd = 20. * log10(*pd);
		pd++;	
		i++;
	}
	pc = &m_pFftOut[0];
	i = 0;
	while(i < 2048)
	{	*pd = _hypot((*pc)[0], (*pc)[1]);
		pc++;
		if(*pd < 1.) *pd = 1.;
		*pd = 20. * log10(*pd);
		pd++;	
		i++;
	}
	m_dFrCent = 0.001 * (double)pscope -> iFrequency;
	m_dFrInc = 1. / (4096. * 0.001 * (double)pscope -> iTimeInc);
	m_iFftPId = pscope -> iPosId;
	DbgPrint(L"TFIndicatorLine +++ Post %ld cCount=%ld fr=%lf res=%lf",
			pscope -> iPosId, pscope -> iCount, 0.001 * (double)pscope -> iFrequency, m_dFrInc);
	unlock();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::mouseMoveEvent(QMouseEvent * event) 
{	int i;
	QPointF fpo = event -> posF();
    Client2Logic(&fpo);
    QPointF pt;

    if(m_MouseAction == mmHand)
	{	pt = m_MousePos - fpo;
		m_s.m_Ofset.setX(pt.x() + m_s.m_Ofset.x());
		m_s.m_Ofset.setY(m_s.m_Ofset.y() - pt.y());
		update();
		return;
	}
    if(m_MouseAction == mmSelect)
	{	pt = m_MousePos - fpo;
		m_dBandSelected = fabs(pt.x());
		update();
		return;
	}
	if(m_MouseAction == mmFormMove)
	{	if(m_FormularPointed)
		{	QPointF pt = m_MousePos;
			Logic2Client(&pt);
			pt -= event -> posF();
			m_FormularPointed -> moveFormuar(pt);
			update();
		}
	}
	m_MousePos = fpo;
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::mousePressEvent(QMouseEvent * event) 
{	int i;
    mmAction ma = m_MouseAction;
	QPointF fpo = event -> posF();
    Client2Logic(&fpo);
    m_MousePos = fpo;

	if(m_MouseAction != mmNone) mouseReleaseEvent(event);
	if(event -> button() & Qt::RightButton)
	{	m_MouseAction = mmSelect;
		m_dFrSelected = m_MousePos.x();
		m_dBandSelected = 0.;
		update();
		return;
	}
	m_FormularPointed = NULL;
	lock();
	
	i = 0;
	while(i < m_FTraceList.count())
	{	TFTrace * pt = m_FTraceList.at(i++);
		int j = 0;
		while(j < pt -> m_pFormularList.count())
		{	TFFormular * pf = pt -> m_pFormularList.at(j++);
			if(pf -> IsInFormular(event -> posF()))
			{	m_FormularPointed = pf;
				unlock();
				m_MouseAction = mmFormMove;
				return;
			}
		}
	}
	unlock();
	m_MouseAction = mmHand;
	setCursor(Qt::OpenHandCursor);
	update();
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::mouseReleaseEvent(QMouseEvent * event) 
{	mmAction  MouseAction = m_MouseAction;
	m_MouseAction = mmNone;

	if(event -> button() & Qt::RightButton)
	{	if(MouseAction == mmSelect)
		{	int iFlags = 0;
			if(m_s.m_EnabMask & ENAB_TA_REQUEST) iFlags |= FRSEL_FLAG_TA;
			((QFIndicatorWidget *)parent()) -> EmitFSelection(m_id, m_dFrSelected,
				m_dBandSelected, m_s.m_iComPoints, m_s.m_dAzFrom, m_s.m_dAzTo, iFlags);
		}
		return;
	}
	if(MouseAction == mmHand)
	{	unsetCursor();
		rebuild();
	}
	m_FormularPointed = NULL;
}
//*****************************************************************************
//
//*****************************************************************************
void TFIndicatorLine::CheckFormularPointed(TFFormular * pf)
{
	if(m_FormularPointed == pf) m_FormularPointed = NULL;
}
//*****************************************************************************
//
//*****************************************************************************
TCheckTread::~TCheckTread()
{
	// DbgPrint(L"Q-FIndicator: CheckTreadThread DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
void TCheckTread::run()
{   unsigned __int64 uqt, uqTime;
	bool bUpdate;	

	while(!m_bTermFlag)
    {   int i = 0;
		while(i++ < 10)
        {	msleep(100);
			if(m_bTermFlag) return;
		}
#ifndef __linux
        GetSystemTimeAsFileTime((FILETIME *)&uqTime);
		uqt = uqTime - 10000000;
#else
        struct timeval tv;
        if (gettimeofday(&tv,NULL)) return;
        uqt = unix_ticks_to_windows_ticks(tv.tv_usec) - 10000000;
#endif
		bUpdate = false;
		m_pFIndicator -> lock();
		i = 0;
		while(i < m_pFIndicator -> m_FTraceList.count())
		{	TFTrace * pt = m_pFIndicator -> m_FTraceList.at(i);
			if(pt -> m_uqLastCheckTime < uqt)
			{	pt -> CheckPoints();
				bUpdate = true;
				if(pt -> IsEmpty())
				{	m_pFIndicator -> m_FTraceList.removeAt(i);
					delete pt;
					continue;
				}
			}
			i++;
		}
		if(m_pFIndicator -> m_FTracePoit -> m_uqLastCheckTime < uqt)
		{	m_pFIndicator -> m_FTracePoit -> CheckPoints();
			bUpdate = true;
		}		
		if(m_pFIndicator -> m_FTraceP0 -> m_uqLastCheckTime < uqt)
		{	m_pFIndicator -> m_FTraceP0 -> CheckPoints();
			bUpdate = true;
		}		
		if(m_pFIndicator -> m_FTraceP1 -> m_uqLastCheckTime < uqt)
		{	m_pFIndicator -> m_FTraceP1 -> CheckPoints();
			bUpdate = true;
		}		
		if(m_pFIndicator -> m_FTraceP2 -> m_uqLastCheckTime < uqt)
		{	m_pFIndicator -> m_FTraceP2 -> CheckPoints();
			bUpdate = true;
		}		
		if(m_pFIndicator -> m_FTraceP3 -> m_uqLastCheckTime < uqt)
		{	m_pFIndicator -> m_FTraceP3 -> CheckPoints();
			bUpdate = true;
		}		
		m_pFIndicator -> unlock();
		if(bUpdate) m_pFIndicator -> update();
	}
}
