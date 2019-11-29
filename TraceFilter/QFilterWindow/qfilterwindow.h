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

public:
    void showStopper();
    void initComponents();

private:
    QList<QObject*> m_qlObjects;
    QStopper *m_pStopper;
    QAction *settingsAct;

    QRect m_qrPropDlgGeo;
    QLabel *lbStatusArea;
    QLabel *lbStatusMsg;
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
