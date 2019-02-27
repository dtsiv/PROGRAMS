#ifndef QCONSOLEWIDGET_H
#define QCONSOLEWIDGET_H

#include <QtCore>
#include <QWidget>

#include "qconsolewidget_global.h"

class QDomDocument;
class QMessage;
class QColor;
class QMutex;
class QPainter;
class QPaintEvent;
class TStyle;
class TPaintTread;
//*****************************************************************************
//
//*****************************************************************************
class QCONSOLEWIDGET_EXPORT QConsoleWidget : public QWidget 
{
	Q_OBJECT

public:
	QConsoleWidget(QWidget * parent);
	~QConsoleWidget();
	void lock(void);
	void unlock(void);
	void getPropPages(QList<QWidget *> * pwl, QWidget * pParent);
	Q_INVOKABLE void initialize(QDomDocument * pDomProp);
	Q_INVOKABLE void getConfiguration(QDomDocument * pDomProp);
	Q_INVOKABLE void addMessage(QString qsMes);
	Q_INVOKABLE void addMessage(int iStyle, QDateTime qDateTime, QString qsMes);
	QStringList about();
	void clear();
	void pause(bool bPause);
	void sizechanged();
	void StartLoging(void);
	void StopLoging(void);
	void CalcSize(void);

signals:
	void SizeChanged(QSize size);
	void SetStatusInfo1(QString qsMessage);

public slots:

public:
	QList<TStyle *> m_StyleList;
	QColor m_BkgColor;
	int m_iSpasing;
	bool m_bPause;
	QString m_LogDir;
	QString m_LogFile;
	int m_iFileNum;
	bool m_bLogEnab;
	QTextStream m_LogStream;

private:
	QMutex m_Lock;
	QList<QMessage *> m_MessageList;
	QList<QMessage *> m_DrawList;
	TPaintTread * m_PaintThread;
	QSize m_CurSize;
	void draw(QPainter * pp);
	bool m_bLogingOn;
	QFile m_fLogFile;
	quint64 m_uqLastPaintTime;

protected:
    void paintEvent(QPaintEvent * event);
};

#endif // QCONSOLEWIDGET_H



