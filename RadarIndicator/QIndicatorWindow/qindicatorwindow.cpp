#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

QIndicatorWindow::QIndicatorWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
      , m_pSqlModel(NULL)
      , m_pTargetsMap(NULL)
      , m_pPoi(NULL)
      , m_pStopper(NULL)
      , settingsAct(NULL) {
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

    m_pTargetsMap = new QTargetsMap(this);
    if (m_pTargetsMap) m_qlObjects << qobject_cast<QObject *> (m_pTargetsMap);

    m_pPoi = new QPoi(this);
    if (m_pPoi) m_qlObjects << qobject_cast<QObject *> (m_pPoi);

    m_qfPeleng.setFileName("peleng.dat");
    // connect simulation timer
    QObject::connect(&m_simulationTimer,SIGNAL(timeout()),SLOT(onSimulationTimeout()));
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

    if (m_qfPeleng.isOpen()) m_qfPeleng.close();
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
        if (m_pSqlModel->execQuery()) m_simulationTimer.start(100);
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
void QIndicatorWindow::onSimulationTimeout() {
    quint64 iRecId;
    int iStrob;
    int iBeamCountsNum;
    qint64 iTimeStamp;
    static bool bStarted=false;

    // get next strob record guid from DB
    if (!m_pSqlModel->getStrobRecord(iRecId, iStrob, iBeamCountsNum, iTimeStamp)) {
        qDebug() << "getStrobRecord failed";
        m_simulationTimer.stop();
        return;
    }
    // raw data array
    QByteArray baSamples;
    // list linked data records for beams
    QList<QByteArray> ql_baTarForBeam;
    QList<int> ql_nTargets;
    for (int iBeam=0; iBeam<QPOI_NUMBER_OF_BEAMS; iBeam++) {
        baSamples.clear();
        if (!m_pSqlModel->getBeamData(iRecId, iBeam, baSamples)) {
            qDebug() << "getBeamData failed";
            m_simulationTimer.stop();
            return;
        }

        ql_baTarForBeam.append(QByteArray());
        ql_nTargets.append(0);
        if (ql_baTarForBeam.size()-1>iBeam || ql_nTargets.size()-1>iBeam) { // redundant check of list size
            qDebug() << "list size mismatch";
            m_simulationTimer.stop();
            return;
        }
        QByteArray baStrTargets;
        int nTargets;
        if (!m_pPoi->detectTargets(baSamples, baStrTargets, nTargets)) {
            // qDebug() << "detectTargets failed: " << iStrob << " " << iBeam;
            continue;
        }
        ql_nTargets[iBeam] = nTargets;
        ql_baTarForBeam[iBeam] = baStrTargets;
        if (baStrTargets.size()!=nTargets*sizeof(struct QPoi::sTarget)) {
            qDebug() << "struct size mismatch: " << baStrTargets.size() << " " << nTargets*sizeof(struct QPoi::sTarget);
            m_simulationTimer.stop();
            return;
        }
        //for (int i=0; i< qlTgts.size(); i++) {
        //    m_pTargetsMap->addTargetMarker(new QTargetMarker(qlTgts.at(i),QString::number(iBeam)));
        //}
    }
    QTextStream tsPeleng(&m_qfPeleng);
    if (!bStarted) {
        m_qfPeleng.resize(0);
        m_qfPeleng.open(QIODevice::ReadWrite);
        if (m_qfPeleng.isOpen()) {
            tsPeleng << QString("StrobNo").rightJustified(10);
            for (int iBeam1=0; iBeam1<QPOI_NUMBER_OF_BEAMS-1; iBeam1++) {
                for (int iBeam2=iBeam1+1; iBeam2<QPOI_NUMBER_OF_BEAMS; iBeam2++) {
                    tsPeleng << "\t" << QString("%1-%2").arg(iBeam1).arg(iBeam2).rightJustified(10);
                }
            }
            tsPeleng << endl;
        }
        bStarted=true;
    }
    if (m_qfPeleng.isOpen()) tsPeleng << QString("%1").arg(iStrob).rightJustified(10);
    for (int iBeam1=0; iBeam1<QPOI_NUMBER_OF_BEAMS-1; iBeam1++) {
        for (int iBeam2=iBeam1+1; iBeam2<QPOI_NUMBER_OF_BEAMS; iBeam2++) {
            int nTargets1 = ql_nTargets[iBeam1];
            int nTargets2 = ql_nTargets[iBeam2];
            // qDebug() << iBeam1 << "-" << iBeam2 << ": " << nTargets1 << ", " << nTargets2;
            for (int iTarget1=0; iTarget1<nTargets1; iTarget1++) {
                for (int iTarget2=0; iTarget2<nTargets2; iTarget2++) {
                    struct QPoi::sTarget *pTarData1 = (struct QPoi::sTarget *)ql_baTarForBeam[iBeam1].data()+iTarget1;
                    struct QPoi::sTarget *pTarData2 = (struct QPoi::sTarget *)ql_baTarForBeam[iBeam2].data()+iTarget2;
                    QPoint qpTar1=pTarData1->qp_rep;
                    QPoint qpTar2=pTarData2->qp_rep;
                    if ((qpTar1-qpTar2).manhattanLength() < QPOI_MAXIMUM_TG_MISMATCH) { // Targets must be close
                        double dM2Tar1=pTarData1->y2mc_rep;
                        double dM2Tar2=pTarData2->y2mc_rep;
                        double dFrac=(dM2Tar1-dM2Tar2)/(dM2Tar1+dM2Tar2);
                        QPointF qpfAvr = (pTarData1->qpf_wei + pTarData2->qpf_wei)/2;
                        QString qsLegend("%1 %2-%3 F:%4");
                        m_pTargetsMap->addTargetMarker(new QTargetMarker(qpfAvr,
                                qsLegend.arg(iStrob).arg(iBeam1).arg(iBeam2).arg(dFrac,0,'f',2)));
                        if (m_qfPeleng.isOpen()) tsPeleng << QString("\t%1").arg(dFrac,10,'f',2);
                    }
                    else {
                        if (m_qfPeleng.isOpen()) tsPeleng << QString("\t%1").arg(QChar(' '),10);
                        qDebug() << "qpTar1!=qpTar2: " << qpTar1 << " " << qpTar2;
                    }
                }
            }
        }
    }
    if (m_qfPeleng.isOpen()) tsPeleng << endl;
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
