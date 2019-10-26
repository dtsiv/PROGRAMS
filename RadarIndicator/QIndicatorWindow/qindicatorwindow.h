#ifndef QINDICATORWINDOW_H
#define QINDICATORWINDOW_H

#include <QMainWindow>
#include <QSplashScreen> 
#include <QtGui>
#include <QtWidgets>
#include <QGLWidget>
#include <QMetaObject>
#include <QtGlobal>

#include "QIniSettings.h"
#include "QPropPages.h"
#include "QSqlModel.h"

class QStopper;

class QIndicatorWindow : public QMainWindow
{
	Q_OBJECT

public:
    QIndicatorWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QIndicatorWindow();

    Q_INVOKABLE void fillTabs(QObject *pPropDlg, QObject *pPropTabs);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

public slots:
    void hideStopper();
    void showDialog();

public:
    void showStopper();
    void initComponents();

private:
    QList<QObject*> m_qlObjects;
    QSqlModel *m_pSqlModel;
    QStopper *m_pStopper;
};

//*****************************************************************************
//
//*****************************************************************************
class QStopper : public QSplashScreen {
public:
	QStopper();
	~QStopper();

protected:
	virtual void drawContents (QPainter *painter);

private:
    QPixmap m_pmStopper;
};

#endif // QINDICATORWINDOW_H
