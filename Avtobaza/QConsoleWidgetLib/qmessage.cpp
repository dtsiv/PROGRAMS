#include "stdafx.h"
#include "QtDebug"
#include "qmessage.h"
#include "proppage.h"
#include "qconsolewidget.h"
#ifdef __linux
#include <wchar.h>
#endif
//*****************************************************************************
//						+++ QMessage +++
//*****************************************************************************
QMessage::QMessage(QConsoleWidget * pOwner) :
	m_pConsole(pOwner), m_iStyleId(0) { }

//*****************************************************************************
//
//*****************************************************************************
QMessage::QMessage(int iStyle, QDateTime qDateTime, QString qsBuf, QConsoleWidget * pOwner) :
	m_pConsole(pOwner), m_qsMessage(qsBuf), m_iStyleId(iStyle) { }

//*****************************************************************************
//
//*****************************************************************************
QMessage::~QMessage() { }

//*****************************************************************************
//
//*****************************************************************************
TStyle * QMessage::getStyle(void)
{
	TStyle * ps;
	int i = 0;
	while(i < m_pConsole -> m_StyleList.count())
	{	ps = m_pConsole -> m_StyleList.at(i++);
		if(ps -> m_iId == m_iStyleId) return(new TStyle(ps));
	}
	return(new TStyle());
}
//*****************************************************************************
//
//*****************************************************************************
QSize QMessage::draw(QPainter * pp, QPoint pt)
{	
	QRect rc0, rc;
	TStyle * ps = getStyle();
	if(ps -> m_bEnabled)
	{	pp -> setFont(ps -> m_Font);
		pp -> setPen(QPen(ps -> m_TextColor));
		QString qs = message();
		rc = pp -> boundingRect(rc0, Qt::TextSingleLine | Qt::TextDontClip, qs);
		pp -> fillRect(QRect(0, pt.y() - rc.height(), rc.width(), rc.height()), QBrush(ps -> m_BkgColor));
		pp -> drawText(pt, qs);
	}
	delete ps;
	return(rc.size());
}
//******************************************************************************
//
//******************************************************************************
QSize QMessage::mesure()
{	
	QRect rc;
	TStyle * ps = getStyle();
	if(ps -> m_bEnabled)
	{	m_pConsole -> setFont(ps -> m_Font);
		QFontMetrics fm = m_pConsole -> fontMetrics();  
		QString qs = message();
		rc = fm.boundingRect(qs);
	}
	delete ps;
	return(rc.size());
}
//******************************************************************************
//
//******************************************************************************
QString QMessage::message() {
    if (!m_qsMessage.isEmpty()) {
	QString qs = m_qDateTime.toString("hh:mm:ss.zzz ");
	return(qs + m_qsMessage);
    }
    else return QString();

}
//******************************************************************************
//
//******************************************************************************
void QMessage::logMessage(void) {
    if (!m_qsMessage.isEmpty()) {
	QString qs = m_qDateTime.toString("hh:mm:ss.zzz ");
	m_pConsole -> m_LogStream << qs + m_qsMessage + "\n";
    }
}
//*****************************************************************************
//
//*****************************************************************************
TPaintTread::~TPaintTread()
{
	DbgPrint(L"QConsoleWidget: TPaintTread DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
void TPaintTread::run()
{   
	while(!m_bTermFlag)
    {   msleep(1000);
		if(m_pConsole -> m_bPause) continue;
		m_pConsole -> CalcSize();
		m_pConsole -> sizechanged();
	}
}


