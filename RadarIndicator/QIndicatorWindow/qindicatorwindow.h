#ifndef QINDICATORWINDOW_H
#define QINDICATORWINDOW_H

#include <QMainWindow>
#include <QSplashScreen> 
#include <QtGui>
#include <QtWidgets>
#include <QGLWidget>
#include <QMetaObject>
#include <QtGlobal>

#include "qinisettings.h"
#include "qproppages.h"
#include "qsqlmodel.h"
#include "qtargetsmap.h"

#define SETTINGS_KEY_GEOMETRY                   "geometry"

#define STOPPER_MIN_DELAY_MSECS                 1000

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
    void onSetup();

public:
    void showStopper();
    void initComponents();

private:
    QList<QObject*> m_qlObjects;
    QSqlModel *m_pSqlModel;
public:
    QTargetsMap *m_pTargetsMap;
private:
    QStopper *m_pStopper;

    QAction *settingsAct;

    QRect qrPropDlgGeo;
    QLabel *lbStatusArea;

};

//*****************************************************************************
//
//*****************************************************************************
class UserControlInputFilter : public QObject {
protected:
    virtual bool eventFilter(QObject*, QEvent*);

public:
    UserControlInputFilter(QIndicatorWindow* pOwner, QObject *pobj=0);

private:
    QIndicatorWindow *m_pOwner;
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
