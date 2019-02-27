#ifndef QLOCALCONTROLQMLWIDGET_P_H
#define QLOCALCONTROLQMLWIDGET_P_H

#include <QtDeclarative>
#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QDomDocument>
#include <QMessageBox>

#include "qlocalcontrolqmlwidget.h"
#include "programmodel.h"

#define VER_MAJOR 2
#define VER_MINOR 4
#define VER_YEAR  2016

class QLocalControlQMLWidgetPrivate
{
	Q_DECLARE_PUBLIC(QLocalControlQMLWidget)

public:
	QLocalControlQMLWidgetPrivate(void) {}

	 // sinchronization
	void lock(void) { m_Lock.lock(); }
	void unlock(void) { m_Lock.unlock(); }

private:
	bool setQmlSource(const QString &localFile);

private:
	QString m_pathToQml;
    ProgramModel * m_pProgramModel;
    QDeclarativeView * m_pDeclarativeView;
	QVBoxLayout * m_pVBoxLayout;
	QLabel * m_pLabelNoQml;
	QMutex m_Lock;

public:
	QLocalControlQMLWidget * q_ptr;

};

#endif // QLOCALCONTROLQMLWIDGET_P_H

