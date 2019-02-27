#include "stdafx.h"
#include <QWidget>
#include <QDomDocument>
#include <QDateTime>
#include <QFile>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QMutex>

#include "proppage.h"
#include "qmessage.h"
#include "qconsolewidget.h"

#define VER_MAJOR 1
#define VER_MINOR 3
#define MESSAGEBUFFERSIZE 80

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//*****************************************************************************
//
//*****************************************************************************
QConsoleWidget::QConsoleWidget(QWidget * parent) : QWidget(parent, 
#ifndef __linux
    Qt::MSWindowsOwnDC
#else // linux
    Qt::Widget
#endif
)
{
	m_BkgColor = Qt::darkYellow;
	m_iSpasing = 1;
	m_bPause = false;
	m_bLogEnab = false;
	m_LogDir = "./";
	m_LogFile = "consolelog_%ld";
	m_iFileNum = 0;
	m_bLogingOn = false;
	m_CurSize = QSize(0, 0);
	m_uqLastPaintTime=QDateTime::currentMSecsSinceEpoch();
	m_PaintThread = new TPaintTread(this);
}
//*****************************************************************************
//
//*****************************************************************************
void QConsoleWidget::lock(void) { m_Lock.lock(); }
//*****************************************************************************
//
//*****************************************************************************
void QConsoleWidget::unlock(void) { m_Lock.unlock(); }
//*****************************************************************************
//
//*****************************************************************************
QConsoleWidget::~QConsoleWidget()
{
	if(m_bLogingOn) StopLoging();
	m_PaintThread -> ThreadStop();
	delete m_PaintThread; 
	while(m_StyleList.count())
	{	TStyle * p = m_StyleList.first();
		m_StyleList.removeFirst();
		delete p;
	}
	while(m_MessageList.count())
	{	QMessage * p = m_MessageList.first();
		m_MessageList.removeFirst();
		delete p;
	}
	while(m_DrawList.count())
	{	QMessage * p = m_DrawList.first();
		m_DrawList.removeFirst();
		delete p;
	}
	DbgPrint(L"QConsoleWidget::DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
QStringList QConsoleWidget::about()
{
	QStringList qsVer;
	QString qs;
#pragma GCC diagnostic ignored "-Wformat"
	qs.sprintf("QConsoleWidgetLib v.%ld.%02ld beta by Tristan 2012", VER_MAJOR, VER_MINOR);
#pragma GCC diagnostic pop
	qsVer << qs;
	return(qsVer);
}
//*****************************************************************************
//
//*****************************************************************************
void QConsoleWidget::initialize(QDomDocument * pDomProp)
{
	QDomNode nd;
	QString qs;
	bool bOk;

	QDomNodeList ndl = pDomProp -> elementsByTagName("Console");	
	if(ndl.count() < 1) return;
	QDomElement eConsole = ndl.at(0).toElement();

	ndl = eConsole.elementsByTagName("BkgColor");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			qs = nd.toText().data();
			m_BkgColor.setRgba(qs.toUInt(&bOk, 16));
		}
	}
	ndl = eConsole.elementsByTagName("Spasing");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			qs = nd.toText().data();
			m_iSpasing = qs.toInt();
		}
	}
	ndl = eConsole.elementsByTagName("LogEnable");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			qs = nd.toText().data();
			m_bLogEnab = (bool)qs.toInt();
		}
	}
	ndl = eConsole.elementsByTagName("LogFileCount");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			qs = nd.toText().data();
			m_iFileNum = qs.toInt();
		}
	}
	ndl = eConsole.elementsByTagName("LogDirectory");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			m_LogDir = nd.toText().data();
		}
	}
	ndl = eConsole.elementsByTagName("LogFile");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			m_LogFile = nd.toText().data();
		}
	}
	ndl = eConsole.elementsByTagName("Styles");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		lock();
		while(m_StyleList.count())
		{	TStyle * p = m_StyleList.first();
			m_StyleList.removeFirst();
			delete p;
		}
		int i = 0;		
		while(i < ndl.count())	
		{	QDomElement eStiles = ndl.at(i++).toElement();
			TStyle * ps = new TStyle();
			QDomNodeList ndl_ = eStiles.elementsByTagName("Id");
			if(ndl_.count() > 0)	
			{	nd = ndl_.at(0);	
				ndl_ = nd.childNodes();
				if(ndl_.count() > 0)				
				{	nd = ndl_.at(0);
					qs = nd.toText().data();
					ps -> m_iId = qs.toInt();
				}
			}
			ndl_ = eStiles.elementsByTagName("BkgColor");
			if(ndl_.count() > 0)	
			{	nd = ndl_.at(0);	
				ndl_ = nd.childNodes();
				if(ndl_.count() > 0)				
				{	nd = ndl_.at(0);
					qs = nd.toText().data();
					ps -> m_BkgColor.setRgba(qs.toUInt(&bOk, 16));
				}
			}
			ndl_ = eStiles.elementsByTagName("TextColor");
			if(ndl_.count() > 0)	
			{	nd = ndl_.at(0);	
				ndl_ = nd.childNodes();
				if(ndl_.count() > 0)				
				{	nd = ndl_.at(0);
					qs = nd.toText().data();
					ps -> m_TextColor.setRgba(qs.toUInt(&bOk, 16));
				}
			}
			ndl_ = eStiles.elementsByTagName("Enable");
			if(ndl_.count() > 0)	
			{	nd = ndl_.at(0);	
				ndl_ = nd.childNodes();
				if(ndl_.count() > 0)				
				{	nd = ndl_.at(0);
					qs = nd.toText().data();
					ps -> m_bEnabled = qs.toUInt();
				}
			}
			ndl_ = eStiles.elementsByTagName("Font");
			if(ndl_.count() > 0)	
			{	nd = ndl_.at(0);	
				ndl_ = nd.childNodes();
				if(ndl_.count() > 0)				
				{	nd = ndl_.at(0);
					qs = nd.toText().data();
					ps -> m_Font.fromString(qs);
				}
			}
			m_StyleList.append(ps);
		}
		unlock();

	}
	if(m_bLogingOn) StopLoging();
	if(m_bLogEnab) StartLoging();
}
//*****************************************************************************
//
//*****************************************************************************
void QConsoleWidget::getConfiguration(QDomDocument * pDomProp)
{
	if(pDomProp == NULL) return;

	int i;
	bool bPiPresent = false;
	bool bComPresent = false;
	QDomElement eParam;
	QString qs;

	QDomNodeList ndl = pDomProp -> childNodes();
	i = 0;
	while(i < ndl.count())
	{	QDomNode nd = ndl.at(i);
		QDomNode::NodeType ndt = nd.nodeType();
#pragma GCC diagnostic ignored "-Wswitch"
		switch(ndt)
		{	case QDomNode::ProcessingInstructionNode:
				bPiPresent = true;
				break;

			case QDomNode::CommentNode:
				bComPresent = true;
				break;

			case QDomNode::ElementNode:
				if(nd.toElement().tagName() == "PARAMETERS")
				eParam = nd.toElement(); 
				break;
		}
#pragma GCC diagnostic pop
		i++;
	}
	if(!bPiPresent)		
	{	QDomProcessingInstruction pi;
		pi = pDomProp -> createProcessingInstruction("xml", "version='1.0' encoding='UTF-8");
		pDomProp -> appendChild(pi); 
	}
	if(!bComPresent)
	{	QDomComment com;
		com = pDomProp -> createComment("QRmo parameters store file");
		pDomProp -> appendChild(com); 
	}
	if(eParam.isNull())
	{	eParam = pDomProp -> createElement("PARAMETERS");
		pDomProp -> appendChild(eParam);
	}
	QDomNodeList nl = eParam.elementsByTagName("Console");
	i = 0;
	while(i < nl.count()) eParam.removeChild(nl.at(i++));
	
	QDomElement eConsole = pDomProp -> createElement("Console");
	QDomElement e;

	e = pDomProp -> createElement("BkgColor");
	QDomText t = pDomProp -> createTextNode(qs.setNum((unsigned int)m_BkgColor.rgba(), 16));
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("Spasing");
	t = pDomProp -> createTextNode(qs.setNum(m_iSpasing));
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("LogEnable");
	t = pDomProp -> createTextNode(qs.setNum((int)m_bLogEnab));
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("LogFileCount");
	t = pDomProp -> createTextNode(qs.setNum(m_iFileNum));
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("LogDirectory");
	t = pDomProp -> createTextNode(m_LogDir);
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("LogFile");
	t = pDomProp -> createTextNode(m_LogFile);
	e.appendChild(t);
	eConsole.appendChild(e);

	e = pDomProp -> createElement("Styles");
	i = 0;
	lock();
	while(i < m_StyleList.count())
	{	TStyle * ps = m_StyleList.at(i++);
		QDomElement e_;
		QDomElement eStile = pDomProp -> createElement("Style");

		e_ = pDomProp -> createElement("Id");
		t = pDomProp -> createTextNode(qs.setNum(ps -> m_iId));
		e_.appendChild(t);
		eStile.appendChild(e_);
		e_ = pDomProp -> createElement("BkgColor");
		t = pDomProp -> createTextNode(qs.setNum((unsigned int)ps -> m_BkgColor.rgba(), 16));
		e_.appendChild(t);
		eStile.appendChild(e_);
		e_ = pDomProp -> createElement("TextColor");
		t = pDomProp -> createTextNode(qs.setNum((unsigned int)ps -> m_TextColor.rgba(), 16));
		e_.appendChild(t);
		eStile.appendChild(e_);
		e_ = pDomProp -> createElement("Enable");
		t = pDomProp -> createTextNode(qs.setNum(ps -> m_bEnabled));
		e_.appendChild(t);
		eStile.appendChild(e_);
		e_ = pDomProp -> createElement("Font");
		t = pDomProp -> createTextNode(ps -> m_Font.toString());
		e_.appendChild(t);
		eStile.appendChild(e_);
		e.appendChild(eStile);
	}
	unlock();
	eConsole.appendChild(e);
	eParam.appendChild(eConsole);
}
//*****************************************************************************
//
//*****************************************************************************
void QConsoleWidget::getPropPages(QList<QWidget *> * pwl, QWidget * pParent)
{
	QWidget * pwg = (QWidget *)new ConsolePropPage(pParent, this);
	pwl -> append(pwg);
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::addMessage(QString qsMes) {	
	addMessage(0, QDateTime::currentDateTime(), qsMes);
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::addMessage(int iStyle, QDateTime qDateTime, QString qsMes)
{
	TStyle * ps;
	QMessage * pm = new QMessage(iStyle, qDateTime, qsMes, this);
	if(m_bLogingOn) pm -> logMessage();
	
	lock();
	bool bFound = false;
	int i = 0;
	while(i < m_StyleList.count())
	{	ps = m_StyleList.at(i++);
		if(ps -> m_iId == iStyle)
		{	bFound = true;
			break;
		}
	}
	if(!bFound)
	{	ps = new TStyle();
		ps -> m_iId = iStyle;
		m_StyleList.append(ps);
	}	
	if(m_bPause)
	{	m_MessageList.append(pm);
		if(m_MessageList.count() > MESSAGEBUFFERSIZE)
		{	QMessage * p = m_MessageList.first();
			m_MessageList.removeFirst();
			delete p;
		}
		unlock();	
		return;
	}
	m_DrawList.append(pm);
	if(m_DrawList.count() > MESSAGEBUFFERSIZE)
	{	QMessage * p = m_DrawList.first();
		m_DrawList.removeFirst();
		delete p;
	}
	unlock();	
//	update();
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::CalcSize()
{	
	QSize size;
	int iy = 20, ix = 0;;
	int i = 0;
	lock();	
	while(i < m_DrawList.count())
	{	QMessage * pm  = m_DrawList.at(i++);
		size = pm -> mesure();
		if(size.isNull()) continue;
		iy += m_iSpasing;
		iy += size.height();
		if(ix < size.width()) ix = size.width();
	}
	m_CurSize = QSize(ix, iy);
	unlock();	
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::sizechanged()
{	
	unsigned __int64 uqTime = QDateTime::currentMSecsSinceEpoch();
	uqTime -= m_uqLastPaintTime;
	double dt = 1.e-3 * (double)uqTime;

	if(minimumSize() != m_CurSize)
	{	setMinimumSize(m_CurSize);
		emit SizeChanged(m_CurSize);
	}
	if(dt > 2.)
	{	//DbgPrint(L"++ Time sins last Paint %0.3lf", dt);
		update();
	}
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::clear()
{
	lock();	
	while(m_MessageList.count())
	{	QMessage * p = m_MessageList.first();
		m_MessageList.removeFirst();
		delete p;
	}
	while(m_DrawList.count())
	{	QMessage * p = m_DrawList.first();
		m_DrawList.removeFirst();
		delete p;
	}
	unlock();	
	update();

}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::paintEvent(QPaintEvent * event)
{	QPainter painter;

    m_uqLastPaintTime=QDateTime::currentMSecsSinceEpoch();
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush br(m_BkgColor);
    painter.fillRect(event -> rect(), br);
    draw(&painter);
    painter.end();
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::pause(bool bPause)
{
	m_bPause = bPause;
	if(!m_bPause)
	{	lock();	
		while(m_MessageList.count())
		{	QMessage * pm = m_MessageList.first();
			m_MessageList.removeFirst();
			m_DrawList.append(pm);
			if(m_DrawList.count() > MESSAGEBUFFERSIZE)
			{	QMessage * p = m_DrawList.first();
				m_DrawList.removeFirst();
				delete p;
			}
		}
		unlock();
	}
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::draw(QPainter * pp)
{	
	
	QSize size;
	int iy = 20;
	
	lock();
	int i = 0;
	while(i < m_DrawList.count())
	{	QMessage * pm  = m_DrawList.at(i++);
		size = pm -> draw(pp, QPoint(0, iy));
		if(size.isNull()) continue;
		iy += m_iSpasing;
		iy += size.height();
	}
	unlock();
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::StartLoging() {
    #define FILENAME_MAXLEN 255
    QString qs;

    char buf[FILENAME_MAXLEN+1];
    ::snprintf(buf, FILENAME_MAXLEN, m_LogFile.toLocal8Bit(), m_iFileNum);
    QString qsFileName = m_LogDir + "/" + QString::fromLocal8Bit(buf) + ".txt";
    m_fLogFile.setFileName(qsFileName);

    bool bRet = m_fLogFile.open(QIODevice::WriteOnly | QIODevice::Text);
    if(!bRet) {	
        qs = "Open failed: file " + qsFileName;
        emit SetStatusInfo1(qs);
        m_bLogEnab = false;
        return;
    }
    m_LogStream.setDevice(&m_fLogFile);
    qs = "Start loging to file " + qsFileName;
    emit SetStatusInfo1(qs);
    m_iFileNum++;

    qs = QDateTime::currentDateTime().toString("dd.MM.yyyy start loging at hh:mm:ss.zzz\n");
    m_LogStream << qs;
    m_bLogingOn = true;
}
//******************************************************************************
//
//******************************************************************************
void QConsoleWidget::StopLoging()
{
	m_bLogingOn = false;
	m_LogStream.flush();
	m_fLogFile.close();
	
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
