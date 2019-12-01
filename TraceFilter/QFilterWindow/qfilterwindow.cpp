#include "qfilterwindow.h"
#include "qexceptiondialog.h"
#include "qgeoutils.h"

#include "nr.h"
using namespace std;

QFilterWindow::QFilterWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
      , m_pStopper(NULL) {
    setWindowIcon(QIcon(QPixmap(":/Resources/tracefilter.ico")));
    m_plbStatusMsg=new QLabel(QString::fromLocal8Bit(CONN_STATUS_DISCONN));
    statusBar()->addPermanentWidget(m_plbStatusMsg,1);
    m_plbReferenceInfo=new QLabel(QString::fromLocal8Bit("Press Control-P for settings"));
    statusBar()->addPermanentWidget(m_plbReferenceInfo);

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
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QFilterWindow::~QFilterWindow() {
    // settings object and main window adjustments
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QRect qrCurGeometry=geometry();
    QString qsEncodedGeometry = QSerial(qrCurGeometry).toBase64();
    iniSettings.setValue(SETTINGS_KEY_GEOMETRY, qsEncodedGeometry);

    // list of components for PropPages
    for (int i=0; i<m_qlObjects.size(); i++) {
        QObject *pObj=m_qlObjects.at(i);
        if (pObj) delete pObj; // need to check destructor, check order of objects in QList!
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::setStatusMessage(QString qsMsg) {
    m_plbStatusMsg->setText(qsMsg);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::showStopper() {
    this->setVisible(false);
    m_pStopper = new QStopper;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::initComponents() {
    QTime qtCurr = QTime::currentTime();

    // create instances of components
    m_pSqlModel = new QSqlModel(this);
    if (m_pSqlModel) m_qlObjects << qobject_cast<QObject *> (m_pSqlModel);

    m_pGeoUtils = new QGeoUtils;
    if (m_pGeoUtils) m_qlObjects << qobject_cast<QObject *> (m_pGeoUtils);

    // create widgets
    QDockWidget *dock = new QDockWidget("QTARGETSMAP_DOC_CAPTION");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    // MapWidget *pMapWidget = m_pTargetsMap->getMapInstance();
    // if (pMapWidget) {
    //     pMapWidget->setFocusPolicy(Qt::StrongFocus);
    //     dock->setWidget(pMapWidget);
    // }
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // time eater to display the stopper window
    while(qtCurr.msecsTo(QTime::currentTime())<STOPPER_MIN_DELAY_MSECS) {
        qApp->processEvents();
    }

    /*
    pj_Set_Geocentric_Parameters( &TdCord::gi, KRASOVSKY_MAJOR_SEMIAXIS, KRASOVSKY_MINOR_SEMIAXIS);
    QFile qfGeo("geoparams.txt");
    qfGeo.open(QIODevice::ReadWrite);
    QTextStream tsGeo(&qfGeo);
    tsGeo.setRealNumberNotation(QTextStream::FixedNotation);
    tsGeo.setRealNumberPrecision(40);
    tsGeo << "TdCord::gi.Geocent_a=" << TdCord::gi.Geocent_a << endl;
    tsGeo << "TdCord::gi.Geocent_b=" << TdCord::gi.Geocent_b << endl;
    tsGeo << "TdCord::gi.Geocent_a2=" << TdCord::gi.Geocent_a2 << endl;
    tsGeo << "TdCord::gi.Geocent_b2=" << TdCord::gi.Geocent_b2 << endl;
    tsGeo << "TdCord::gi.Geocent_e2=" << TdCord::gi.Geocent_e2 << endl;
    tsGeo << "TdCord::gi.Geocent_ep2=" << TdCord::gi.Geocent_ep2 << endl;
    */

    QTimer::singleShot(0,this,SLOT(hideStopper()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::hideStopper() {
    if (m_pStopper) {
        delete m_pStopper;
        m_pStopper = NULL;
    }
    setVisible(true);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::onSetup() {
    static bool bInSetup=false;
    if (!bInSetup) {
        bInSetup=true;
        settingsAct->setEnabled(false);
        QPropPages dlgPropPages(this);
        if (!m_qrPropDlgGeo.isEmpty()) dlgPropPages.setGeometry(m_qrPropDlgGeo);
        dlgPropPages.exec();
        m_qrPropDlgGeo=dlgPropPages.geometry();
        settingsAct->setEnabled(true);
        bInSetup=false;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::onTestPgConnection() {
    QObject *pobj = QObject::sender();
    QPropPages *pPropPages = qobject_cast<QPropPages *>(pobj);
    if (!pPropPages || !m_pSqlModel) {
        setStatusMessage("Could not start test");
        return;
    }

    QString qsErrMsg;
    bool bOk = m_pSqlModel->testPgConnection(
                qsErrMsg,
                pPropPages->m_pleDBDatabaseHostname->text(),
                pPropPages->m_pleDBDatabaseName->text(),
                pPropPages->m_pleDBDatabaseUser->text(),
                pPropPages->m_pleDBDatabasePassword->text(),
                pPropPages->m_pleDBDatabaseEncoding->text());
    // set status bar message depending on result
    if (bOk) {
        setStatusMessage("Postgres test succeeded");
    }
    else {
        setStatusMessage(qsErrMsg);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::fillTabs(QObject *pPropDlg, QObject *pPropTabs) {
    for (int i=0; i<m_qlObjects.size(); i++) {
        QMetaObject::invokeMethod(m_qlObjects.at(i),"addTab",Q_ARG(QObject *,pPropDlg), Q_ARG(QObject *,pPropTabs), Q_ARG(int, i));
        qobject_cast<QTabWidget*>(pPropTabs)->widget(i)->setAutoFillBackground(true);
   }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QFilterWindow::propChanged(QObject *pPropDlg) {
    for (int i=0; i<m_qlObjects.size(); i++) {
        QMetaObject::invokeMethod(m_qlObjects.at(i),"propChanged",Q_ARG(QObject *,pPropDlg));
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStopper::QStopper()
       : QSplashScreen(QPixmap(), Qt::WindowStaysOnTopHint)
       , m_pmStopper(QPixmap(":/Resources/filterstopper.png")) {
    setPixmap(m_pmStopper);
    resize(m_pmStopper.width(),m_pmStopper.height());
    QDesktopWidget *desktop = QApplication::desktop();
    QRect qrScrGeo = desktop->screenGeometry(desktop->primaryScreen());
    move((qrScrGeo.width()-m_pmStopper.width())/2,(qrScrGeo.height()-m_pmStopper.height())/2);
    show();
    repaint();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QStopper::drawContents ([[maybe_unused]] QPainter *painter) {
    // qDebug() << "Inside draw contents";
	// painter->drawPixmap(0,0,m_pmHarddisk);
    //painter->setBrush(QBrush(Qt::blue));
    //painter->setPen(Qt::blue);
    //painter->setFont(QFont("Arial", 16));
    //painter->drawText(QRect(0,m_pmHarddisk.height()-30,m_pmHarddisk.width(),30),Qt::AlignCenter,"Connecting to DB ...");
//	painter->fillRect(10,10,100,100,Qt::SolidPattern);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStopper::~QStopper() {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UserControlInputFilter::UserControlInputFilter(QFilterWindow *pOwner, QObject *pobj /*=0*/)
    : QObject(pobj), m_pOwner(pOwner) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*virtual*/ bool UserControlInputFilter::eventFilter([[maybe_unused]]QObject *pobj, QEvent *pe) {
    QEvent::Type eventType = pe->type();
    // user input events
    if (eventType == QEvent::KeyPress) {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pe;
        // qDebug() << "key= " << pKeyEvent->key();
        // qDebug() << "pKeyEvent->modifiers()= " << QString::number((int)pKeyEvent->modifiers(),16);
        if (pKeyEvent->key()==Qt::Key_Return) {
            // block all other events
            return true;
        }
        else if ((pKeyEvent->key()==Qt::Key_P)
               && (pKeyEvent->modifiers() & Qt::ControlModifier)) {
            m_pOwner->onSetup();
            return true;
        }
        else if (pKeyEvent->key()==Qt::Key_Escape) {
            // m_pOwner->closeView();
            return true;
        }
        else if ((pKeyEvent->key()==Qt::Key_Q || pKeyEvent->key()==Qt::Key_X)
               && (pKeyEvent->modifiers() & Qt::ControlModifier || pKeyEvent->modifiers() & Qt::AltModifier)) {
            qApp->quit();
            return true;
        }
    }
    // allow all other events
    return false;
}
