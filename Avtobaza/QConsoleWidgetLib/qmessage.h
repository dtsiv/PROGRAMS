#ifndef QMESSAGE_H
#define QMESSAGE_H

#include <QtCore>
#include <QPainter>

#include <math.h>

#include "codograms_and_qavtctrl.h"

class QConsoleWidget;
class TStyle;
//*****************************************************************************
//
//*****************************************************************************
class QMessage
{

public:
	QMessage(QConsoleWidget * pOwner);
	QMessage(int iStyle, QDateTime qDateTime, QString qsBuf, QConsoleWidget * pOwner);
	~QMessage();
	QSize draw(QPainter * pp, QPoint pt);
	QSize mesure();
	void logMessage();

public:
	QDateTime m_qDateTime;

private:
	QConsoleWidget * m_pConsole;
	QString m_qsMessage;
	int m_iStyleId;
	QString message();
	TStyle * getStyle(void);
};

//*****************************************************************************
//
//*****************************************************************************
class TPaintTread : public QThread
{
public:
	TPaintTread(QConsoleWidget * pOwner) : QThread()
	{	m_pConsole = pOwner;
		m_bTermFlag = false;
		start();
	}
	~TPaintTread();
	bool ThreadStop()
	{	m_bTermFlag = true;
		return(wait(60000));
	}

public:

private:
	void run();
	QConsoleWidget * m_pConsole;
	volatile bool m_bTermFlag;
};
#endif // QMESSAGE_H



