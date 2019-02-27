#include "qledindicator.h"
#include "rmoexception.h"
#include <QPainter>
#include <QTimer>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLedIndicator::QLedIndicator(bool bBlinking /* =false */, int iHeight /*=16*/, QWidget *parent /* =0 */ , Qt::WindowFlags f /* =0 */)
	: QLabel(parent,f)
 , m_qlsState(QLedIndicator::StateOff) 
 , m_pTimer(NULL) 
 , m_ppmCurrent(NULL) {

	QMap<int,QString> qmIconFiles;
	qmIconFiles[qlc_gray]  = "16x16-gray.png";
	qmIconFiles[qlc_green] = "16x16-green.png";
	qmIconFiles[qlc_red]   = "16x16-red.png";
	qmIconFiles[qlc_blue]  = "16x16-blue.png";
	qmIconFiles[qlc_yellow]= "16x16-yellow.png";
		
	QMapIterator<int,QString> i(qmIconFiles);
	while (i.hasNext()) {
	    i.next();
        QPixmap tmpPixmap(QString(":/Resources/")+i.value());
		if (tmpPixmap.isNull()) qDebug() << QString("Bad resource: "+i.value());
		m_qmPixmaps[i.key()]=new QPixmap(tmpPixmap.scaledToHeight(iHeight));
		// qDebug() << pIcon->actualSize(QSize(QLEDINDICATOR_SIZE,QLEDINDICATOR_SIZE));
	}
	setColorForState(m_qlsState);
	m_pTimer=new QTimer(this);
	QObject::connect(m_pTimer,SIGNAL(timeout()),SLOT(onTimeout()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLedIndicator::~QLedIndicator() {
	QMapIterator<int,QPixmap*> i(m_qmPixmaps);
	while (i.hasNext()) {
	    i.next();
		QPixmap *pPixmap = i.value(); 
		if (pPixmap) delete pPixmap;
	}
	if (m_pTimer) {
		m_pTimer->stop();
		delete (m_pTimer);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::setColorForState(QLedIndicator::QLedState state) {
	m_ppmCurrent=NULL;
	switch(state) {
		case StateOff: 
        case StateNum0:          // general gray
			m_ppmCurrent=m_qmPixmaps[qlc_gray]; break;
		case StateReady: 
        case StateConnected:
		case TempOk:
        case StateNum1:          // general green
            m_ppmCurrent=m_qmPixmaps[qlc_green]; break;
		case StateErrCon:
		case TempAbove60:
        case StateNum2:          // general yellow
			m_ppmCurrent=m_qmPixmaps[qlc_yellow]; break;
        case StateDisconnected:
		case StateError: 
		case TempAbove75:
        case StateNum3:          // general red
            m_ppmCurrent=m_qmPixmaps[qlc_red]; break;
		case StateIrradiationOn: 
        case StateNum4:          // general red blinking
            m_pTimer->stop();
            m_pTimer->start(QLEDINDICATOR_TIMEOUT);
			m_ppmCurrent=m_qmPixmaps[qlc_red]; break;
		case TempBelow5:
        case StateNum5:          // general blue
			m_ppmCurrent=m_qmPixmaps[qlc_blue]; break;
//		default:
//			m_ppmCurrent=m_qmPixmaps[qlc_gray]; break;
	}
	if (!m_ppmCurrent) {
		QTimer::singleShot(0,this,SLOT(errorReport()));
		return;
	}
	setPixmap(*m_ppmCurrent);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::errorReport() {
	throw RmoException("QLedIndicator error!");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::onTimeout() {
	if (m_qlsState!=StateIrradiationOn && m_qlsState!=StateNum4) return;
	if (!isEnabled()) {
		setColorForState(StateOff);
		return;
	}
	static bool bOn=false;
	if (bOn) {
		setColorForState(m_qlsState);
	}
	else {
		setColorForState(StateOff);
	}
	bOn=!bOn;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::setTemperature(int iTemp) {
	QLedState state=StateDummy;
	if (iTemp<5) state=TempBelow5;
	else if (iTemp<60) state=TempOk;
	else if (iTemp<75) state=TempAbove60; 
	else state=TempAbove75;
	setState(state);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::setState(QLedState qlsState) {
    setColorForState(m_qlsState=qlsState);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QLedIndicator::setNumState(int iState) {
    if (iState>StateNum5) throw RmoException("Bad QLedIndicator state");
    setColorForState(m_qlsState=(QLedState)iState);
}
