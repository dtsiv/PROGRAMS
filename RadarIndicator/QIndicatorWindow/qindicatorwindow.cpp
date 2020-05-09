#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

#include "nr.h"
using namespace std;

QIndicatorWindow::QIndicatorWindow(QWidget *parent, Qt::WindowFlags flags)
        : QMainWindow(parent, flags)
          , m_pSqlModel(NULL)
          , m_pRegFileParser(NULL)
          , m_pTargetsMap(NULL)
          , m_pPoi(NULL)
          , m_pStopper(NULL)
          , settingsAct(NULL)
          , m_pParseMgr(NULL)
          , m_pSimuMgr(NULL)
          , m_pNoiseMapMgr(NULL)
          , m_bParsingInProgress(false)
          , m_bGenerateNoiseMapInProgress(false) {
    setWindowIcon(QIcon(QPixmap(":/Resources/spear.ico")));
    lbStatusArea=new QLabel(QString::fromLocal8Bit(CONN_STATUS_DISCONN));
    statusBar()->addPermanentWidget(lbStatusArea,1);
    lbStatusMsg=new QLabel(QString::fromLocal8Bit("Press Control-P for settings"));
    statusBar()->addPermanentWidget(lbStatusMsg);

    // settings object and main window adjustments
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(SETTINGS_KEY_GEOMETRY,QSerial(QRect(200, 200, 200, 200)).toBase64());
    QString qsEncodedGeometry = iniSettings.value(SETTINGS_KEY_GEOMETRY,scRes).toString();
    // qDebug() << scRes << " err=" << QIniSettings::INIT_ERROR << " notf=" << QIniSettings::KEY_NOT_FOUND << " def=" << QIniSettings::READ_DEFAULT << " valid=" << QIniSettings::READ_VALID;
    bool bOk;
    QRect qrGeometry = QSerial(qsEncodedGeometry).toRect(&bOk);
    // qDebug() << "qrGeometry=" << qrGeometry << " bOk=" << bOk;
    if (bOk) setGeometry(qrGeometry);

    // catch user input enter event
    QCoreApplication *pCoreApp = QCoreApplication::instance();
    pCoreApp->installEventFilter(new UserControlInputFilter(this,pCoreApp));

    // create actions
    settingsAct = new QAction(QIcon(":/Resources/settings.ico"), tr("Settings"), this);
    settingsAct->setShortcut(QKeySequence("Ctrl+P"));
    settingsAct->setStatusTip(QString::fromLocal8Bit("Settings"));
    settingsAct->setText(QString::fromLocal8Bit("Settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(onSetup()));

    // create instances of components
    m_pSqlModel = new QSqlModel();
    if (m_pSqlModel) m_qlObjects << qobject_cast<QObject *> (m_pSqlModel);

    QString qsRegFileName;
    m_pRegFileParser = new QRegFileParser(qsRegFileName);
    if (m_pRegFileParser) m_qlObjects << qobject_cast<QObject *> (m_pRegFileParser);

    m_pTargetsMap = new QTargetsMap(this);
    if (m_pTargetsMap) m_qlObjects << qobject_cast<QObject *> (m_pTargetsMap);

    m_pPoi = new QPoi(this);
    if (m_pPoi) m_qlObjects << qobject_cast<QObject *> (m_pPoi);

    // connect simulation timer
    m_simulationTimer.setInterval(m_pTargetsMap->m_uTimerMSecs);
    QObject::connect(&m_simulationTimer,SIGNAL(timeout()),SLOT(onSimulationTimeout()));

    m_pParseMgr = new QParseMgr(this);
    m_pSimuMgr = new QSimuMgr(this);
    m_pNoiseMapMgr = new QNoiseMapMgr(this);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QIndicatorWindow::~QIndicatorWindow() {
    // settings object and main window adjustments
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QRect qrCurGeometry=geometry();
    QString qsEncodedGeometry = QSerial(qrCurGeometry).toBase64();
    // qDebug() << geometry() << " " << qsEncodedGeometry;
    iniSettings.setValue(SETTINGS_KEY_GEOMETRY, qsEncodedGeometry);

    for (int i=0; i<m_qlObjects.size(); i++) {
        QObject *pObj=m_qlObjects.at(i);
        if (pObj) delete pObj; // need to check destructor, check order of objects in QList!
    }
    if (m_pStopper)   { delete m_pStopper; m_pStopper=NULL; }
    if (m_pParseMgr)  { delete m_pParseMgr; m_pParseMgr=NULL; }
    if (m_pSimuMgr)   { delete m_pSimuMgr; m_pSimuMgr=NULL; }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::initComponents() {
    QTime qtCurr = QTime::currentTime();
    QDockWidget *dock = new QDockWidget(QTARGETSMAP_DOC_CAPTION);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    MapWidget *pMapWidget = m_pTargetsMap->getMapInstance();
    if (pMapWidget) {
        pMapWidget->setFocusPolicy(Qt::StrongFocus);
        dock->setWidget(pMapWidget);
    }
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // connect to sqlite DB
    if (m_pSqlModel->openDatabase()) {
        lbStatusArea->setText(CONN_STATUS_SQLITE);
        if (m_pSqlModel->execQuery()) m_simulationTimer.start();
    }

    // time eater to display the stopper window
    while(qtCurr.msecsTo(QTime::currentTime())<STOPPER_MIN_DELAY_MSECS) {
        qApp->processEvents();
    }
    QTimer::singleShot(0,this,SLOT(hideStopper()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::hideStopper() {
    if (m_pStopper) {
        delete m_pStopper;
        m_pStopper = NULL;
    }
    setVisible(true);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::onSetup() {
    static bool bInSetup=false;
    if (!bInSetup) {
        bInSetup=true;
        settingsAct->setEnabled(false);
        QPropPages dlgPropPages(this);
        if (!m_qrPropDlgGeo.isEmpty()) dlgPropPages.setGeometry(m_qrPropDlgGeo);
        dlgPropPages.exec();
        m_qrPropDlgGeo=dlgPropPages.geometry();
        // if DB is not open - cleanup & reconnect
        if (!m_pSqlModel->isDBOpen()) {
            qDebug() << "onSetup(): DB is not open - cleanup & reconnect";
            // set disconnected status
            lbStatusArea->setText(CONN_STATUS_DISCONN);
            // stop simulation timer
            if (m_simulationTimer.isActive()) {
                qDebug() << "onSetup(): stopping simulation timer";
                m_simulationTimer.stop();
            }
            // clear stale target markers
            qDebug() << "onSetup(): clearMarkers()";
            m_pTargetsMap->clearMarkers();
            // reconnect to DB
            if (m_pSqlModel->openDatabase()) {
                if (m_pSqlModel->execQuery()) {
                    qDebug() << "DB reopened";
                    lbStatusArea->setText(CONN_STATUS_SQLITE);
                    // reconnected -- restart simulation
                    m_simulationTimer.start();
                }
            }
            else {
                qDebug() << "Failed to open DB " << m_pSqlModel->getDBFileName();
            }
        }
        // adapt new settings if dialog was accepted
        if (dlgPropPages.result() == QDialog::Accepted) {
            // restart timer with new interval (msec)
            m_simulationTimer.start(m_pTargetsMap->m_uTimerMSecs);
        }
        // setup was finished and results applied
        settingsAct->setEnabled(true);
        bInSetup=false;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::fillTabs(QObject *pPropDlg, QObject *pPropTabs) {
    for (int i=0; i<m_qlObjects.size(); i++) {
        QMetaObject::invokeMethod(m_qlObjects.at(i),"addTab",Q_ARG(QObject *,pPropDlg), Q_ARG(QObject *,pPropTabs), Q_ARG(int, i));
        qobject_cast<QTabWidget*>(pPropTabs)->widget(i)->setAutoFillBackground(true);
   }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::propChanged(QObject *pPropDlg) {
    for (int i=0; i<m_qlObjects.size(); i++) {
        QMetaObject::invokeMethod(m_qlObjects.at(i),"propChanged",Q_ARG(QObject *,pPropDlg));
    }
}
