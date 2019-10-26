#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

QIndicatorWindow::QIndicatorWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
      , m_pSqlModel(NULL)
      , m_pStopper(NULL)
      , settingsAct(NULL) {
    setWindowIcon(QIcon(QPixmap(":/Resources/spear.ico")));
    lbStatusArea=new QLabel(QString::fromLocal8Bit(CONN_STATUS_DISCONN));
    statusBar()->addPermanentWidget(lbStatusArea,1);

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
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::showStopper() {
    this->setVisible(false);
    m_pStopper = new QStopper;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::initComponents() {
    QTime qtCurr = QTime::currentTime();
    while(qtCurr.msecsTo(QTime::currentTime())<3000) {
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
        if (!qrPropDlgGeo.isEmpty()) dlgPropPages.setGeometry(qrPropDlgGeo);
        dlgPropPages.exec();
        qrPropDlgGeo=dlgPropPages.geometry();
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStopper::QStopper()
: QSplashScreen(QPixmap(), Qt::WindowStaysOnTopHint)
, m_pmStopper(QPixmap(":/Resources/stopper_screen.png")) {
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
UserControlInputFilter::UserControlInputFilter(QIndicatorWindow *pOwner, QObject *pobj /*=0*/)
    : QObject(pobj), m_pOwner(pOwner) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*virtual*/ bool UserControlInputFilter::eventFilter([[maybe_unused]] QObject *pobj, QEvent *pe) {
    QEvent::Type eventType = pe->type();
    // user input events
    if (eventType == QEvent::KeyPress) {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pe;
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
        else if ((pKeyEvent->key()==Qt::Key_Plus || pKeyEvent->key()==Qt::Key_Minus
               || pKeyEvent->key()==Qt::Key_Equal|| pKeyEvent->key()==Qt::Key_Bar )
               && (pKeyEvent->modifiers() & Qt::ControlModifier)) {
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
