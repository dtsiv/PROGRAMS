#include "QIndicatorWindow.h"
#include "QExceptionDialog.h"

QIndicatorWindow::QIndicatorWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
      , m_pSqlModel(NULL)
      , m_pStopper(NULL) {
    setWindowIcon(QIcon(QPixmap(":/Resources/spear.ico")));

    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue("BlaBlaBla","No meaning at all!");

    m_pSqlModel = new QSqlModel();
    if (m_pSqlModel) m_qlObjects << qobject_cast<QObject *> (m_pSqlModel);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QIndicatorWindow::~QIndicatorWindow() {
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
	throw RmoException("Test exception");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::showDialog() {
    QPropPages dlgPropPages(this);
    dlgPropPages.exec();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QIndicatorWindow::fillTabs(QObject *pPropDlg, QObject *pPropTabs) {
    for (int i=0; i<m_qlObjects.size(); i++) {
        QMetaObject::invokeMethod(m_qlObjects.at(i),"addTab",Q_ARG(QObject *,pPropDlg), Q_ARG(QObject *,pPropTabs), Q_ARG(int, i));
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
void QStopper::drawContents (QPainter *painter) {
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
