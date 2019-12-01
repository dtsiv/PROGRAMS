#ifndef QFILTERWINDOW_H
#define QFILTERWINDOW_H

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
#include "qgeoutils.h"

#define SETTINGS_KEY_GEOMETRY                   "geometry"

#define STOPPER_MIN_DELAY_MSECS                 3000

class QStopper;

class QFilterWindow : public QMainWindow
{
    Q_OBJECT

public:
    QFilterWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QFilterWindow();

    Q_INVOKABLE void fillTabs(QObject *pPropDlg, QObject *pPropTabs);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

public slots:
    void hideStopper();
    void onSetup();
    void onTestPgConnection();
    void setStatusMessage(QString qsMsg);

public:
    void showStopper();
    void initComponents();

private:    // list of components
    QList<QObject*> m_qlObjects;
    QSqlModel *m_pSqlModel;
    QGeoUtils *m_pGeoUtils;

private:
    QStopper *m_pStopper;
    QAction *settingsAct;

    QRect m_qrPropDlgGeo;
    QLabel *m_plbStatusMsg;
    QLabel *m_plbReferenceInfo;
};

//*****************************************************************************
//
//*****************************************************************************
class UserControlInputFilter : public QObject {
protected:
    virtual bool eventFilter(QObject*, QEvent*);

public:
    UserControlInputFilter(QFilterWindow* pOwner, QObject *pobj=0);

private:
    QFilterWindow *m_pOwner;
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

#endif // QFILTERWINDOW_H
