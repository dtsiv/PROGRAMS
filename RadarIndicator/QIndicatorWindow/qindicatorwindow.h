#ifndef QINDICATORWINDOW_H
#define QINDICATORWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtWidgets>
#include <QGLWidget>
#include <QMetaObject>
#include <QtGlobal>

#include "qinisettings.h"
#include "qproppages.h"
#include "qsqlmodel.h"
#include "qregfileparser.h"
#include "qtargetsmap.h"
#include "qpoi.h"
#include "qstopper.h"
#include "usercontrolinputfilter.h"
#include "qparsemgr.h"
#include "qsimumgr.h"

#define SETTINGS_KEY_GEOMETRY                   "geometry"
#define SETTINGS_KEY_SHUFFLE                    "useShuffle"

#define SIMULATION_TIMER_INTERVAL               100

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
    void onSimulationTimeout();
    void onParseDataFile();
    void onUpdateProgressBar(double dCurr);

signals:
    void updateProgressBar(double dCurr);

public:
    void showStopper();
    void initComponents();

private:
    QList<QObject*> m_qlObjects;
    QSqlModel *m_pSqlModel;
    QRegFileParser *m_pRegFileParser;
    QTargetsMap *m_pTargetsMap;
    QPoi *m_pPoi;

private:
    QStopper *m_pStopper;

    QAction *settingsAct;
private:
    QRect m_qrPropDlgGeo;
    QLabel *lbStatusArea;
    QLabel *lbStatusMsg;
    QTimer m_simulationTimer;
    bool m_bUseShuffle;
    QParseMgr *m_pParseMgr;
    QSimuMgr *m_pSimuMgr;
public:
    bool m_bParsingInProgress;

friend class QParseMgr;
friend class QSimuMgr;
};

#endif // QINDICATORWINDOW_H
