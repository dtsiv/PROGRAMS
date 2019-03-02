#include "qspear.h"
#include <vlc-qt/WidgetVideo.h>
#include <vlc-qt/Common.h>
#include <vlc-qt/Instance.h>
#include <vlc-qt/Media.h>
#include <vlc-qt/MediaPlayer.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void adjustGridLayout(QGridLayout *pLayout) {
    int iWidth=0;
    for (int i=0; i<pLayout->rowCount(); i++) {
        for (int j=0; j<pLayout->columnCount(); j++) {
            QLayoutItem *pItem=pLayout->itemAtPosition(i,j);
            if (!pItem) continue;
            QWidget *pWidget = pItem->widget();
            QLayout *pCellLayout = pItem->layout();
            if (pWidget) iWidth=qMax(iWidth,pWidget->minimumWidth());
            else if (pCellLayout) iWidth=qMax(iWidth,pCellLayout->minimumSize().width());
        }
    }
    for (int j=0; j<pLayout->columnCount(); j++) {
        pLayout->setColumnMinimumWidth(j,iWidth);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutJustirovka() {
    QList<QPushButton*> qlButtons;
    int iWidth;
	QHBoxLayout *pHLayout;

    pHLayout=new QHBoxLayout;
    QGridLayout *pglJustirovka=new QGridLayout;
    m_pqlRotSpeed = new QLabel("");
    pglJustirovka->addWidget(m_pqlRotSpeed,0,0,1,3,Qt::AlignHCenter);
    m_pslRotationSpeed = new QSlider(Qt::Horizontal);
    m_pslRotationSpeed->setMaximum(5);
    m_pslRotationSpeed->setMinimum(0);
    int iSpeed=m_pModel->rotSpeed();
    m_pslRotationSpeed->setValue(iSpeed);
    m_pqlRotSpeed->setText(QString("%1: %2").arg(tr("RotSpeed")).arg(iSpeed));
    m_pslRotationSpeed->setOrientation(Qt::Horizontal);
    m_pslRotationSpeed->setMaximumHeight(100);
    pglJustirovka->addWidget(m_pslRotationSpeed,1,0,1,3,Qt::AlignHCenter);
    //pglJustirovka->addItem(new QSpacerItem(1,50),1,0);
    m_ppbRotLeft = new QPushButton(tr("RotLeft"));
    pglJustirovka->addWidget(m_ppbRotLeft,3,0,Qt::AlignHCenter);
    m_ppbRotUp = new QPushButton(tr("RotUp"));
    pglJustirovka->addWidget(m_ppbRotUp,2,1,Qt::AlignHCenter);
    m_ppbRotDown = new QPushButton(tr("RotDown"));
    pglJustirovka->addWidget(m_ppbRotDown,4,1,Qt::AlignHCenter);
    m_ppbRotRight = new QPushButton(tr("RotRight"));
    pglJustirovka->addWidget(m_ppbRotRight,3,2,Qt::AlignHCenter);
    m_ppbRotStop = new QPushButton(tr("RotStop"));
    pglJustirovka->addWidget(m_ppbRotStop,3,1,Qt::AlignHCenter);
    qlButtons<<m_ppbRotLeft<<m_ppbRotRight<<m_ppbRotUp<<m_ppbRotDown<<m_ppbRotStop;
    iWidth=0;
    for (int i=0; i<qlButtons.size(); i++) {
        QPushButton *ppb=qlButtons.at(i);
        iWidth=qMax(iWidth,ppb->fontMetrics().width(ppb->text()));
    }
    for (int i=0; i<qlButtons.size(); i++) {
        qlButtons.at(i)->setMaximumWidth(iWidth+10);
    }
    pglJustirovka->addItem(new QSpacerItem(1,20),5,0);
    pglJustirovka->addWidget(new QLabel(tr("Uvelichenie:")),6,0,2,2,Qt::AlignHCenter);
    m_ppbZoomIn = new QPushButton(tr("+"));
    m_ppbZoomIn->setMaximumWidth(30);
    pglJustirovka->addWidget(m_ppbZoomIn,6,2,Qt::AlignHCenter);
    m_ppbZoomOut = new QPushButton(tr("-"));
    m_ppbZoomOut->setMaximumWidth(30);
    pglJustirovka->addWidget(m_ppbZoomOut,7,2,Qt::AlignHCenter);
    pglJustirovka->addItem(new QSpacerItem(1,20),8,0);
    pglJustirovka->addWidget(new QLabel(tr("Sohraniti napr ne:")),9,0,1,3,Qt::AlignLeft);
    m_ppbSaveDirAS=new QPushButton(tr("ant syst"));
    m_ppbSaveDirCan=new QPushButton(tr("orudie"));
    qlButtons.clear();
    qlButtons<<m_ppbSaveDirCan<<m_ppbSaveDirAS;
    iWidth=0;
    for (int i=0; i<qlButtons.size(); i++) {
        QPushButton *ppb=qlButtons.at(i);
        iWidth=qMax(iWidth,ppb->fontMetrics().width(ppb->text()));
    }
    for (int i=0; i<qlButtons.size(); i++) {
        qlButtons.at(i)->setFixedWidth(iWidth+20);
    }
    pglJustirovka->addWidget(m_ppbSaveDirAS,10,0,1,3,Qt::AlignHCenter);
    pglJustirovka->addWidget(m_ppbSaveDirCan,11,0,1,3,Qt::AlignHCenter);
    pglJustirovka->setRowStretch(12,1);
	pHLayout->addLayout(pglJustirovka);

    m_pVideo = new VlcWidgetVideo(ui.centralWidget);
    m_pVideo->setObjectName(QString::fromUtf8("video"));
    m_pInstance = new VlcInstance(VlcCommon::args(), this);
    m_pPlayer = new VlcMediaPlayer(m_pInstance);
    m_pPlayer->setVideoWidget(m_pVideo);
    m_pVideo->setMediaPlayer(m_pPlayer);
    pHLayout->addWidget(m_pVideo);

	pHLayout->setStretch(1,1);
    return(pHLayout);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutTopIndicators() {
    int iPointSize=24;
    QFont qfo;
	qfo.setPointSize(iPointSize);
	m_pledReady = new QLedIndicator(false,32);
	m_pqlReady = new QLabel(tr("NotReady"));
    m_pqlReady->setMinimumWidth(qMax(QFontMetrics(qfo).width(tr("NotReady")),QFontMetrics(qfo).width(tr("Ready"))));
	adjustLabel(m_pqlReady,iPointSize);
    m_pqlReady->setAlignment(Qt::AlignLeft);
	QFormLayout *pFormLayout1=new QFormLayout;
	pFormLayout1->addRow(m_pledReady,m_pqlReady);
    m_pledIrradiation = new QLedIndicator(true,32);
	m_pqlIrradiation=new QLabel(tr("Irradiation off"));
	adjustLabel(m_pqlIrradiation,iPointSize);
    m_pqlIrradiation->setAlignment(Qt::AlignLeft);
    m_pqlIrradiation->setMinimumWidth(qMax(QFontMetrics(qfo).width(tr("Irradiation off")),QFontMetrics(qfo).width(tr("Irradiation on"))));
	QFormLayout *pFormLayout2=new QFormLayout;
	pFormLayout2->addRow(m_pledIrradiation,m_pqlIrradiation);
	QHBoxLayout *pHLayout=new QHBoxLayout;
    pHLayout->addStretch(1);
    pHLayout->addLayout(pFormLayout1);
    pHLayout->addItem(new QSpacerItem(20,1));
    pHLayout->addLayout(pFormLayout2);
    pHLayout->addStretch(1);
    return pHLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutCoordinaty() {
    QGridLayout *pLayout=new QGridLayout;
    QLabel *plbAntenna=new QLabel(tr("VynosnayaAntenna"));
    plbAntenna->setWordWrap(true);
    plbAntenna->setMaximumWidth(plbAntenna->fontMetrics().width(plbAntenna->text())*2/3);
    pLayout->addWidget(plbAntenna,1,0);
    pLayout->addWidget(new QLabel(tr("Cannon")),2,0);
    pLayout->addWidget(new QLabel(tr("Shirota")),0,1);
    pLayout->addWidget(new QLabel(tr("Dolgota")),0,2);
    pLayout->addWidget(new QLabel(tr("Vysota")),0,3);
    pLayout->addItem(new QSpacerItem(10,1),0,4);
    m_ppbSaveCoord=new QPushButton(tr("Sohraniti"));
    pLayout->addWidget(m_ppbSaveCoord,3,0,1,4);
    QStringList qslCoord = m_pModel->getCoordinates();
    m_pleAntB = new QRmoLineEdit(qslCoord.at(0),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleAntB,1,1);
    m_pleAntL = new QRmoLineEdit(qslCoord.at(1),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleAntL,1,2);
    m_pleAntH = new QRmoLineEdit(qslCoord.at(2),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleAntH,1,3);
    m_pleCannonB = new QRmoLineEdit(qslCoord.at(3),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleCannonB,2,1);
    m_pleCannonL = new QRmoLineEdit(qslCoord.at(4),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleCannonL,2,2);
    m_pleCannonH = new QRmoLineEdit(qslCoord.at(5),QRmoLineEdit::ctFixedPoint);
    pLayout->addWidget(m_pleCannonH,2,3);

    return pLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutRezhimRaboty() {
	QGridLayout *pLayout=new QGridLayout;
    m_ppbRazvernuti = new QPushButton(tr("Razvernuti"));
	pLayout->addWidget(m_ppbRazvernuti,0,0);
    m_ppbFK = new QPushButton(tr("FK"));
	pLayout->addWidget(m_ppbFK,1,0);
    m_ppbPrepareData = new QPushButton(tr("PodgotovkaDannyh"));
	pLayout->addWidget(m_ppbPrepareData,2,0);
    m_ppbStrelba = new QPushButton(tr("Strelba"));
	pLayout->addWidget(m_ppbStrelba,3,0);
	m_pqlWorkTime = new QLabel("0:00");
	adjustLabel(m_pqlWorkTime,24,"9999:99");
	pLayout->addWidget(m_pqlWorkTime,0,1,2,1,Qt::AlignHCenter|Qt::AlignVCenter);
    m_ppbObrabotka = new QPushButton(tr("ObrabotkaDannyh"));
	pLayout->addWidget(m_ppbObrabotka,2,1);
    m_ppbSvernuti = new QPushButton(tr("Svernuti"));
	pLayout->addWidget(m_ppbSvernuti,3,1);
    QList<QPushButton*> qlButtons;
    qlButtons<<m_ppbRazvernuti<<m_ppbFK<<m_ppbPrepareData<<m_ppbStrelba<<m_ppbObrabotka<<m_ppbSvernuti;
    int iWidth=0;
    for (int i=0; i<qlButtons.size(); i++) {
        QPushButton *ppb=qlButtons.at(i);
        iWidth=qMax(iWidth,ppb->fontMetrics().width(ppb->text()));
    }
    for (int i=0; i<qlButtons.size(); i++) {
        qlButtons.at(i)->setMaximumWidth(iWidth+10);
    }
//    adjustGridLayout(pLayout);
    return pLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutControli() {
	QGridLayout *pLayout=new QGridLayout;
	QFormLayout *pFormLayout;
	pFormLayout=new QFormLayout;
	m_pledAntennaSys = new QLedIndicator;
	pFormLayout->addRow(m_pledAntennaSys,new QLabel(tr("Antenna system")));
    pLayout->addLayout(pFormLayout,0,0,1,2,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledUD01 = new QLedIndicator;
	pFormLayout->addRow(m_pledUD01,new QLabel(tr("UD01")));
    pLayout->addLayout(pFormLayout,1,0,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pled4PP01_1 = new QLedIndicator;
	pFormLayout->addRow(m_pled4PP01_1,new QLabel(tr("4PP01_1")));
    pLayout->addLayout(pFormLayout,2,0,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pled4PP01_2 = new QLedIndicator;
	pFormLayout->addRow(m_pled4PP01_2,new QLabel(tr("4PP01_2")));
    pLayout->addLayout(pFormLayout,3,0,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pled4PP01_3 = new QLedIndicator;
	pFormLayout->addRow(m_pled4PP01_3,new QLabel(tr("4PP01_3")));
    pLayout->addLayout(pFormLayout,4,0,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pled4PP01_4 = new QLedIndicator;
	pFormLayout->addRow(m_pled4PP01_4,new QLabel(tr("4PP01_4")));
    pLayout->addLayout(pFormLayout,5,0,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pled3PP01 = new QLedIndicator;
	pFormLayout->addRow(m_pled3PP01,new QLabel(tr("3PP01")));
    pLayout->addLayout(pFormLayout,1,1,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledCS01_1 = new QLedIndicator;
	pFormLayout->addRow(m_pledCS01_1,new QLabel(tr("CS01_1")));
    pLayout->addLayout(pFormLayout,2,1,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledCS01_2 = new QLedIndicator;
	pFormLayout->addRow(m_pledCS01_2,new QLabel(tr("CS01_2")));
    pLayout->addLayout(pFormLayout,3,1,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledGG01 = new QLedIndicator;
	pFormLayout->addRow(m_pledGG01,new QLabel(tr("GG01")));
    pLayout->addLayout(pFormLayout,4,1,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledSU01 = new QLedIndicator;
	pFormLayout->addRow(m_pledSU01,new QLabel(tr("SU01")));
    pLayout->addLayout(pFormLayout,5,1,Qt::AlignLeft);
	pFormLayout=new QFormLayout;
	m_pledTemperature = new QLedIndicator;
	pFormLayout->addRow(m_pledTemperature,new QLabel(tr("Temperature")));
    pLayout->addItem(new QSpacerItem(1,10),6,0);
    pLayout->addLayout(pFormLayout,7,0,1,2,Qt::AlignLeft);
    adjustGridLayout(pLayout);
    return pLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutUpravlenie() {
	QHBoxLayout *pLayout=new QHBoxLayout;
	m_ppbIrradiation=new QPushButton(tr("Irradiation"));
	m_ppbIrradiation->setCheckable(true);
	pLayout->addWidget(m_ppbIrradiation);
    return pLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutChastota() {
	QFormLayout *pFormLayout=new QFormLayout;
    m_pleFreq = new QRmoLineEdit(m_pModel->qsFreq(),QRmoLineEdit::ctFrequency);
	QLabel *pqlFreq=new QLabel(tr("MHz"));
	pqlFreq->setMaximumWidth(pqlFreq->minimumSizeHint().width());
	pFormLayout->addRow(m_pleFreq,pqlFreq);
    m_ppbApplyFreq = new QPushButton(tr("Apply frequency"));
	m_ppbApplyFreq->setMaximumWidth(m_ppbApplyFreq->minimumSizeHint().width());
    QHBoxLayout *pHBoxLayout=new QHBoxLayout;
    pHBoxLayout->addLayout(pFormLayout);
    pHBoxLayout->addItem(new QSpacerItem(10,1));
	pHBoxLayout->addWidget(m_ppbApplyFreq);
    pHBoxLayout->addStretch(1);
    return pHBoxLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QLayout * QSpear::helperLayoutPolozhenieAS() {
    m_pqlASAzim = new QLabel(m_pModel->qsAzim());
	adjustLabel(m_pqlASAzim,18,"+89.9");
	m_pqlASElev = new QLabel(m_pModel->qsElev());
	adjustLabel(m_pqlASElev,18,"+89.9");
	QGridLayout *pLayout=new QGridLayout;
    pLayout->addWidget(new QLabel(tr("AzimuthAS")),0,0,Qt::AlignRight | Qt::AlignVCenter);
    pLayout->addWidget(m_pqlASAzim,0,1,Qt::AlignLeft | Qt::AlignVCenter);
    QLabel *qlbElevationAS=new QLabel(tr("ElevationAS"));
    qlbElevationAS->setWordWrap(true);
    qlbElevationAS->setMaximumWidth(qlbElevationAS->fontMetrics().width(qlbElevationAS->text())*2/3);
    pLayout->addWidget(qlbElevationAS,0,2,Qt::AlignRight | Qt::AlignVCenter);
    pLayout->addWidget(m_pqlASElev,0,3,Qt::AlignLeft | Qt::AlignVCenter);
    return pLayout;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::arrangeControls() {

	//============== define contents of the central Widget ==============
	QHBoxLayout *pTmpLayout=new QHBoxLayout;

	//================ three vertical columns with groupboxes =======================
    m_hblAllColumns=new QHBoxLayout;

	//================ left column =======================
    m_vblLeftColumn = new QVBoxLayout;

	m_pgbUpravlenie=new QGroupBox(tr("Upravlenie"));
	m_pgbUpravlenie->setLayout(helperLayoutUpravlenie());

	m_pgbRezhimRaboty = new QGroupBox(tr("RezhimRaboty"));
	m_pgbRezhimRaboty->setLayout(helperLayoutRezhimRaboty());

	m_pgbKontroli=new QGroupBox(tr("Kontroli"));
	m_pgbKontroli->setLayout(helperLayoutControli());

    m_pgbPolozhenieAS = new QGroupBox(tr("PolozhenieAS"));
    m_pgbPolozhenieAS->setLayout(helperLayoutPolozhenieAS());

    m_vblLeftColumn->setAlignment(Qt::AlignTop);
    m_vblLeftColumn->addWidget(m_pgbUpravlenie);
    m_pgbFrequency = new QGroupBox(tr("FrequencyGroupBox"));
    m_pgbFrequency->setLayout(helperLayoutChastota());
    m_vblLeftColumn->addWidget(m_pgbFrequency);
    m_vblLeftColumn->addLayout(pTmpLayout);
    m_vblLeftColumn->addWidget(m_pgbRezhimRaboty);
    m_vblLeftColumn->addWidget(m_pgbKontroli);
    m_vblLeftColumn->addWidget(m_pgbPolozhenieAS);

    m_pgbCoordinates = new QGroupBox(tr("CoordinatesGroupBox"));
    m_pgbCoordinates->setLayout(helperLayoutCoordinaty());
    m_vblLeftColumn->addWidget(m_pgbCoordinates);

    m_pgbExit = new QGroupBox(tr("ExitButton"));
    pTmpLayout=new QHBoxLayout;
    m_ppbExit = new QPushButton(tr("ExitButton"));
    pTmpLayout->addStretch(1);
    pTmpLayout->addWidget(m_ppbExit);
    pTmpLayout->addStretch(1);
    pTmpLayout->setStretch(1,2);
    m_pgbExit->setLayout(pTmpLayout);
    m_vblLeftColumn->addWidget(m_pgbExit);
    m_vblLeftColumn->addStretch();
    m_hblAllColumns->addLayout(m_vblLeftColumn);

	//================ central column =======================
    m_vblCentralColumn = new QVBoxLayout;

	//--- console ---
	QGroupBox *pgbConsole=new QGroupBox(tr("ConsoleGroupbox"));
	m_pteConsole = new QPlainTextEdit;
	m_pteConsole->setFocusPolicy(Qt::NoFocus);
	QVBoxLayout *pVLayout09=new QVBoxLayout;
    m_ppbClearMessages=new QPushButton(tr("ClearMessages"));
    pVLayout09->addWidget(m_ppbClearMessages);
	pVLayout09->addWidget(m_pteConsole);
	pgbConsole->setLayout(pVLayout09);
    m_vblCentralColumn->addWidget(pgbConsole);
    m_hblAllColumns->addLayout(m_vblCentralColumn);

	//================ right column =======================
    m_vblRightColumn = new QVBoxLayout;
	m_pgbJustirovka=new QGroupBox(tr("Justirovka"));
    m_pgbJustirovka->setLayout(helperLayoutJustirovka());
	m_vblRightColumn->addWidget(m_pgbJustirovka);
	m_vblRightColumn->addStretch();
    m_hblAllColumns->addLayout(m_vblRightColumn);
    m_hblAllColumns->setStretch(2,1);

    //================== finalize main widget layout ===========
    ui.verticalLayout->addLayout(helperLayoutTopIndicators());
    ui.verticalLayout->addLayout(m_hblAllColumns);

	// actions
	exitAct = new QAction(QIcon(":/Resources/exit.png"), tr("Exit"), this);
	// no default action in Windows! exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	exitAct->setStatusTip(tr("Exit"));
	exitAct->setText(tr("Exit"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(QIcon(":/Resources/about.png"), tr("About"), this);
	aboutAct->setShortcuts(QKeySequence::HelpContents);
    aboutAct->setStatusTip(tr("About"));
	aboutAct->setText(tr("About"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(onAbout()));

	ui.mainToolBar->addAction(exitAct);
	ui.mainToolBar->addAction(aboutAct);
	ui.mainToolBar->setFloatable(false);
	ui.mainToolBar->setMovable(false);
	ui.mainToolBar->setAllowedAreas(Qt::TopToolBarArea);

	m_pqlStatusMsg=new QLabel(tr("Connecting..."));
	statusBar()->addPermanentWidget(m_pqlStatusMsg);

	// catch user input enter event
	QCoreApplication *pCoreApp = QCoreApplication::instance();
	pCoreApp->installEventFilter(new UserControlInputFilter(this,pCoreApp));

	QObject::connect(m_ppbIrradiation,SIGNAL(toggled(bool)),SLOT(onCommand()));
	QObject::connect(m_ppbFK,SIGNAL(clicked()),SLOT(onCommand()));
	QObject::connect(m_ppbRazvernuti,SIGNAL(clicked()),SLOT(onCommand()));
	QObject::connect(m_ppbSvernuti,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbExit,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbSaveCoord,SIGNAL(clicked()),SLOT(onCommand()));
	QObject::connect(m_ppbApplyFreq,SIGNAL(clicked()),SLOT(onCommand()));
	QObject::connect(m_ppbStrelba,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_pslRotationSpeed,SIGNAL(sliderReleased()),SLOT(onCommand()));
    QObject::connect(m_ppbSaveDirAS,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbSaveDirCan,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbRotLeft,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbRotRight,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbRotUp,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbRotDown,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbRotStop,SIGNAL(clicked()),SLOT(onCommand()));
    QObject::connect(m_ppbClearMessages,SIGNAL(clicked()),SLOT(onCommand()));

	QTimer::singleShot(0,this,SLOT(onStartup()));
}
