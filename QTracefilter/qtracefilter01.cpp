#include "QTraceFilter.h"
#include "qgenerator.h"
#include "qindicator.h"
#include "qvoiprocessor.h"

#define QTRACEFILTER_DUMMY_DEFAULT    999999

quint32 QTraceFilter::m_uTimeSlice = QTRACEFILTER_DUMMY_DEFAULT;
quint32 QTraceFilter::m_uTickIdx = QTRACEFILTER_DUMMY_DEFAULT;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::arrangeControls() {
	ui.setupUi(this);
    setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));

    // data source selector
	QHBoxLayout *pHLayout08=new QHBoxLayout;
	pHLayout08->addWidget(new QLabel("Data source:"));
	prbPoiteDB=new QRadioButton("POITE database");
	pHLayout08->addWidget(prbPoiteDB);
	prbGen=new QRadioButton("simulator");
	pHLayout08->addWidget(prbGen);
	m_pbgTrajectoryType = new QButtonGroup;
	m_pbgTrajectoryType->addButton(prbPoiteDB);
	m_pbgTrajectoryType->addButton(prbGen);

	// POITE db related
	QVBoxLayout *pVLayout09=new QVBoxLayout;
    m_pgbPoiteDb=new QGroupBox("POITE database");
	QHBoxLayout *pHLayout01=new QHBoxLayout;
    pHLayout01->addWidget(new QLabel("Slice"));
	comboTimeSlice=new QComboBox;
    pHLayout01->addWidget(comboTimeSlice);
	horizontalSlider=new QSlider;
	horizontalSlider->setOrientation(Qt::Horizontal);
    pHLayout01->addWidget(horizontalSlider);
	pVLayout09->addLayout(pHLayout01);
	QHBoxLayout *pHLayout02=new QHBoxLayout;
	QLabel *lblTimeSliceSel=new QLabel("Time slice selection:");
	QFont qfTimeSliceSelFont(lblTimeSliceSel->font());
	qfTimeSliceSelFont.setBold(true);
	lblTimeSliceSel->setFont(qfTimeSliceSelFont);
	pHLayout02->addWidget(lblTimeSliceSel);
	lblTime=new QLabel;
	pHLayout02->addWidget(lblTime);
	pHLayout02->addStretch();
	pVLayout09->addLayout(pHLayout02);
    m_pgbPoiteDb->setLayout(pVLayout09);

	// simulator related
	QVBoxLayout *pVLayout10=new QVBoxLayout;
	m_pgbImit=new QGroupBox("Simulator");
	QHBoxLayout *pHLayout06=new QHBoxLayout;
	pHLayout06->addWidget(prbGen);
	pHLayout06->addStretch();
	pVLayout10->addLayout(pHLayout06);
	QHBoxLayout *pHLayout07=new QHBoxLayout;
	m_pcbTraj=new QComboBox();
	QStringList qslItems;
	qslItems.insert((int)QGenerator::LinearTrajectory,"Linear trajectory");
	qslItems.insert((int)QGenerator::LinearWithKink,"Linear trajectory with kink");
	qslItems.insert((int)QGenerator::CircleTrajectory,"Circle trajectory");
    m_pcbTraj->addItems(qslItems);
	pHLayout07->addWidget(new QLabel("Select trajectory type"));
	pHLayout07->addWidget(m_pcbTraj);
    pHLayout07->addStretch();
	pVLayout10->addLayout(pHLayout07);
    m_pgbImit->setLayout(pVLayout10);

	// 2D equation related
	QHBoxLayout *pHLayout09=new QHBoxLayout;
	pHLayout09->addWidget(new QLabel("Skip post id: "));
	m_pbgSkipPost = new QButtonGroup;
	m_pbgSkipPost->setExclusive(false);
	for (int i=1; i<=4; i++) {
		QCheckBox *pcbPost=new QCheckBox(QString::number(i));
		pcbPost->setChecked(QVoiProcessor::m_pbSkipPost[i]);
		pHLayout09->addWidget(pcbPost);
        m_pbgSkipPost->addButton(pcbPost);
        m_pbgSkipPost->setId(pcbPost,i);
	}
	pHLayout09->addStretch();

	// control buttons
	QHBoxLayout *pHLayout04=new QHBoxLayout;
	pHLayout04->addStretch();
	buttonUpdate=new QPushButton("Update");
	pHLayout04->addWidget(buttonUpdate);
	buttonUpdate->setDefault(true);
	buttonClose=new QPushButton("Close");
    pHLayout04->addWidget(buttonClose);

	ui.verticalLayout->addLayout(pHLayout08);
	ui.verticalLayout->addStretch();
	ui.verticalLayout->addWidget(m_pgbPoiteDb);
	ui.verticalLayout->addStretch();
	ui.verticalLayout->addWidget(m_pgbImit);
	ui.verticalLayout->addStretch();
	ui.verticalLayout->addLayout(pHLayout09);
	ui.verticalLayout->addStretch();
	ui.verticalLayout->addLayout(pHLayout04);

	// signal-slot connections
	QObject::connect(horizontalSlider,SIGNAL(valueChanged(int)),SLOT(onValueChanged(int)));
	QObject::connect(buttonUpdate,SIGNAL(clicked()),SLOT(onUpdate()));
	QObject::connect(buttonClose,SIGNAL(clicked()),SLOT(onClosed()));
	QObject::connect(m_pbgTrajectoryType,SIGNAL(buttonClicked(int)),SLOT(onControlToggled()));
	QObject::connect(m_pbgSkipPost,SIGNAL(buttonClicked(int)),SLOT(onPostSkipped(int)));
	QObject::connect(m_pcbTraj,SIGNAL(currentIndexChanged(int)),SLOT(onControlToggled()));

	// actions
	exitAct = new QAction(QIcon(":/Resources/exit.png"), QString::fromLocal8Bit("Exit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(QString::fromLocal8Bit("Exit"));
	exitAct->setText(QString::fromLocal8Bit("Exit"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	aboutAct = new QAction(QIcon(":/Resources/about.png"), QString::fromLocal8Bit("About"), this);
	aboutAct->setShortcuts(QKeySequence::HelpContents);
    aboutAct->setStatusTip(QString::fromLocal8Bit("About"));
	aboutAct->setText(QString::fromLocal8Bit("About"));
    connect(aboutAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	updateAct = new QAction(QIcon(":/Resources/player_play.png"), tr("Update"), this);
	updateAct->setShortcut(QKeySequence(Qt::Key_Enter));
    updateAct->setStatusTip(QString::fromLocal8Bit("Update"));
	updateAct->setText(QString::fromLocal8Bit("Update"));
    connect(updateAct, SIGNAL(triggered()), this, SLOT(onUpdate()));

	settingsAct = new QAction(QIcon(":/Resources/settings.ico"), tr("Settings"), this);
	settingsAct->setShortcut(QKeySequence("Ctrl+P"));
    settingsAct->setStatusTip(QString::fromLocal8Bit("Settings"));
	settingsAct->setText(QString::fromLocal8Bit("Settings"));
    connect(settingsAct, SIGNAL(triggered()), this, SLOT(onSetup()));

	//menus
	fileMenu = menuBar()->addMenu(QString::fromLocal8Bit("&File"));
    fileMenu->addAction(updateAct);
    fileMenu->addAction(settingsAct);
	fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    helpMenu = menuBar()->addMenu(QString::fromLocal8Bit("&Help"));
    helpMenu->addAction(aboutAct);
	ui.mainToolBar->addAction(updateAct);
	ui.mainToolBar->addAction(settingsAct);
	ui.mainToolBar->addAction(exitAct);
	lbStatusArea=new QLabel(QString::fromLocal8Bit(CONN_STATUS_DISCONN));
	statusBar()->addPermanentWidget(lbStatusArea,1);
    lbStatusMsg=new QLabel(QString::fromLocal8Bit("To start press Update"));
	statusBar()->addPermanentWidget(lbStatusMsg);

	m_pPoiModel=new QPoiModel();

	// catch user input enter event
	QCoreApplication *pCoreApp = QCoreApplication::instance();
	pCoreApp->installEventFilter(new UserControlInputFilter(this,pCoreApp));

    // application settings
	QFile qfSettings(QDir::current().absolutePath()+"/QTraceFilter.xml");
    // detect de facto open mode for settings xml file
    QIODevice::OpenMode omSettings=QIODevice::NotOpen;
    if (qfSettings.exists()) {
        if (!qfSettings.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                    | QFileDevice::ReadUser | QFileDevice::WriteUser
                    | QFileDevice::ReadGroup | QFileDevice::WriteGroup
                    | QFileDevice::ReadOther | QFileDevice::WriteOther)) {
            qDebug() << "qfSettings.setPermissions failed!";
        }
        if (qfSettings.open(QIODevice::ReadWrite)) {
            omSettings=QIODevice::ReadWrite;
            qfSettings.close();
        }
        else if (qfSettings.open(QIODevice::ReadOnly)) {
            omSettings=QIODevice::ReadOnly;
            qfSettings.close();
        }
    }
    // regisger xml format
    QSettings::Format XmlFormat = QSerial::registerXmlFormat();
    if (omSettings!=QIODevice::NotOpen && XmlFormat != QSettings::InvalidFormat) {
        QFileInfo fiSettings(qfSettings);
        if (omSettings==QIODevice::ReadOnly) { // for readOnly settings file - use registry
            qDebug() << "omSettings==QIODevice::ReadOnly: possibly CD-ROM?";
            QSettings tmpSettings(fiSettings.absoluteFilePath(),XmlFormat);
            m_pSettings = new QSettings(COMPANY_NAME,APPLICATION_NAME);
            // on fresh OS -- just copy from xml into registry
            if (!m_pSettings->childGroups().contains(SETTINGS_GROUP_NAME)) {
                tmpSettings.beginGroup(SETTINGS_GROUP_NAME);
                m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
                QStringList qslKeys = tmpSettings.allKeys();
                for (int i=0; i<qslKeys.size(); i++) {
                    m_pSettings->setValue(qslKeys.at(i),tmpSettings.value(qslKeys.at(i)));
                }
                m_pSettings->endGroup();
            }
        }
        else { // ReadWrite open mode
            m_pSettings = new QSettings(fiSettings.absoluteFilePath(),XmlFormat);
        }
    }
    else { // something is wrong with file access
        if (XmlFormat == QSettings::InvalidFormat) { // xml format registering failed
            qDebug() << "XmlFormat == QSettings::InvalidFormat";
        }
        else { // no access to xml file
            qDebug() << "no settings xml file found! Using default settings";
        }
        m_pSettings = new QSettings(COMPANY_NAME,APPLICATION_NAME);
    }

	// fill list of defaults
    defaultParameterValues();

	// read parameters from xml file
	readSettings();

	// update front panel controls
	prbGen->setChecked(QVoiProcessor::m_bUseGen);
    prbPoiteDB->setChecked(!QVoiProcessor::m_bUseGen);
	m_pcbTraj->setCurrentIndex((int)QGenerator::m_ttGenTrajectory);
    onControlToggled();

	// size of time slice (min) 
	m_uMaxTick = 0;
	m_qlTimeSlices << 3 << 5 << 10 << 30 << 60 << 100 << 200 << 500 << 1000;
    for (int i=0; i<m_qlTimeSlices.size(); ++i) {
		comboTimeSlice->addItem(QString::number(m_qlTimeSlices.at(i))+" min");
    }
	int iSliceIdx=qBound(0,m_qlTimeSlices.indexOf(m_uTimeSlice),m_qlTimeSlices.count()-1);
	comboTimeSlice->setCurrentIndex(iSliceIdx);
	horizontalSlider->setMinimum(0);
	horizontalSlider->setTickPosition(QSlider::TicksBelow);
	QObject::connect(comboTimeSlice,SIGNAL(currentIndexChanged(int)),SLOT(onSliceSelected(int)));
	QTimer::singleShot(0,this,SLOT(onStartup()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onStartup() {
	if (QVoiProcessor::m_bUseGen == false) { // use data base as data source
        this->setVisible(false);
		QStopper stopperWindow;
		qApp->processEvents();
        QVoiProcessor::m_bUseGen=true;
        QTimer::singleShot(0,this,SLOT(onControlToggled()));
        if (connectToDatabase()) QVoiProcessor::m_bUseGen=false;
        this->setVisible(true);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::displayConnStatus(QPoiModel::POI_DB_ENGINES eCurrEngine) {
	switch (eCurrEngine) {
		case QPoiModel::DB_Engine_Undefined:
			lbStatusArea->setText(QString(CONN_STATUS_DISCONN));
			break;
		case QPoiModel::DB_Engine_Sqlite:
			lbStatusArea->setText(QString(CONN_STATUS_SQLITE));
			break;
		case QPoiModel::DB_Engine_Postgres:
			lbStatusArea->setText(QString(CONN_STATUS_POSTGRES));
			break;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QTraceFilter::connectToDatabase() {
	if (m_dbeEngineSelected==QPoiModel::DB_Engine_Postgres) {
		if (!m_pPoiModel->initPostgresDB(m_qsPgConnStr)) {
			qDebug() << "QTraceFilter::onStartup(): initPostgresDB failed!";
			return false;
		}
	} else if (m_dbeEngineSelected==QPoiModel::DB_Engine_Sqlite) {
		bool bRetVal=false;
		QFile qfSqliteDb(m_qsSqliteFilePath);
		QFileInfo fiSqliteDb(qfSqliteDb);
		if (fiSqliteDb.exists() && fiSqliteDb.isFile()) {
			bRetVal = m_pPoiModel->initSqliteDB(qfSqliteDb);
		}
		else {
			qDebug() << "QTraceFilter::onStartup(): SQLite database file missing: " + m_qsSqliteFilePath;
			throw RmoException(QString("SQLite database file missing: ") + m_qsSqliteFilePath);
			return false;
		}
		if (!bRetVal) {
			qDebug() << "QTraceFilter::onStartup(): SQLite database file fault: " + m_qsSqliteFilePath;
			throw RmoException(QString("SQLite database file fault: ") + m_qsSqliteFilePath);
			return false;
		}
	}
	refreshTimeSlider();
	quint32 uCurrTick = qBound((quint32)0,m_uTickIdx,m_uMaxTick);
	horizontalSlider->setValue(uCurrTick);
	onValueChanged(uCurrTick);
	displayConnStatus(m_dbeEngineSelected);
	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onControlToggled() {
    this->setVisible(true);
	bool bUseGen = prbGen->isChecked();
	if (bUseGen != QVoiProcessor::m_bUseGen) { // DB connection state altered
		m_pPoiModel->releaseDataSource(); // old data source not needed any more
		displayConnStatus(QPoiModel::DB_Engine_Undefined); // future connection state unknown
		if (bUseGen == false) { // DB connection switched on from off
			prbPoiteDB->setChecked(!QVoiProcessor::m_bUseGen);
			prbGen->setChecked(QVoiProcessor::m_bUseGen);
			m_pgbImit->setEnabled(QVoiProcessor::m_bUseGen);
			m_pgbPoiteDb->setEnabled(!QVoiProcessor::m_bUseGen);
			if (!connectToDatabase()) return; 
		}
	}
	// update static flag m_bUseGen and enable interface elements as needed
	QVoiProcessor::m_bUseGen=bUseGen;
    prbPoiteDB->setChecked(!bUseGen);
    prbGen->setChecked(bUseGen);
	m_pgbImit->setEnabled(bUseGen);
	m_pgbPoiteDb->setEnabled(!bUseGen);
	if (QVoiProcessor::m_bUseGen) {
		QGenerator::m_ttGenTrajectory=(QGenerator::TrajectoryTypes)m_pcbTraj->currentIndex();
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onPostSkipped(int id) {
	if (id<0 || id>=sizeof(QVoiProcessor::m_pbSkipPost)/sizeof(bool)) {
		throw RmoException("QTraceFilter::onPostSkipped Invalid post id");
		return;
	}
	QAbstractButton* pcbCurrent=m_pbgSkipPost->button(id);
    QVoiProcessor::m_pbSkipPost[id]=pcbCurrent->isChecked();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::defaultParameterValues() {
    // default values of parameters
	m_qmParamDefaults[SETTINGS_KEY_PGCONN]              = QVariant(QString("host=127.0.0.1 dbname=rmo user=rmo password=123 client_encoding=WIN1251"));
	m_qmParamDefaults[SETTINGS_SQLITE_FILE]             = QVariant(QString("poite20170602.db"));
	m_qmParamDefaults[SETTINGS_KEY_MAINCTRL]            = QVariant(QString("02.06.2017-09.00.22.639-mainctrl.cfg"));
	m_qmParamDefaults[SETTINGS_KEY_DBENGINE]            = QVariant((int)QPoiModel::DB_Engine_Sqlite);
	m_qmParamDefaults[SETTINGS_KEY_FLT_SIGMAS]          = QVariant(10.0e0);
	m_qmParamDefaults[SETTINGS_KEY_FLT_SIGMAM]          = QVariant(18.0e0);
	m_qmParamDefaults[SETTINGS_KEY_FLT_HEIGHT]          = QVariant(5.0e0);
	m_qmParamDefaults[SETTINGS_KEY_FLT_MINSTROB]        = QVariant(100.0e0); // km
	m_qmParamDefaults[SETTINGS_KEY_FLT_STROBSPREAD]     = QVariant(10.0e0); // m/s
	m_qmParamDefaults[SETTINGS_KEY_FLT_CHI2PROB]        = QVariant(0); 
	m_qmParamDefaults[SETTINGS_KEY_TRAJDUR]             = QVariant(30); // min
	m_qmParamDefaults[SETTINGS_KEY_FLT_USE_TOL]         = QVariant(1);
	m_qmParamDefaults[SETTINGS_KEY_FLT_TOLERANCE]       = QVariant(1.0e-3);
	m_qmParamDefaults[SETTINGS_KEY_FLT_MATRELAX]        = QVariant(1.0e3); // sec
	m_qmParamDefaults[SETTINGS_KEY_FLT_USERELAX]        = QVariant(true);
    m_qmParamDefaults[SETTINGS_KEY_FLT_STICKMODES]      = QVariant(true);
    m_qmParamDefaults[SETTINGS_KEY_FLT_RCLUSTER]        = QVariant(50.0e0); // km
    m_qmParamDefaults[SETTINGS_KEY_FLT_CLUSTERSZ]       = QVariant(7); // minimum number of primary points
    m_qmParamDefaults[SETTINGS_KEY_FLT_TRAJTIMEOUT]     = QVariant(100); // minutes to kill traj
    m_qmParamDefaults[SETTINGS_KEY_FLT_TRAJVMAX]        = QVariant(2.0e3); // max velocity (m/s) to kill traj
    m_qmParamDefaults[SETTINGS_KEY_FLT_ESTINIVELO]      = QVariant(true); // estimate ini velocity from pri cluster
    m_qmParamDefaults[SETTINGS_KEY_GEN_SIGMAM]          = QVariant(18.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_DELAY]           = QVariant(30.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_X0]              = QVariant(-50.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_Y0]              = QVariant(-150.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_V0]              = QVariant(100.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_AZ0]             = QVariant(-30.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJ_R]          = QVariant(200.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJ_CW]         = QVariant(true);
	m_qmParamDefaults[SETTINGS_KEY_GEN_HEIGHT]          = QVariant(5.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_KINKTIME]        = QVariant(10.0e0);
	m_qmParamDefaults[SETTINGS_KEY_GEN_KINKANGLE]       = QVariant(30.0e0);
    m_qmParamDefaults[SETTINGS_KEY_GEN_CLUSTERSZ]       = QVariant(7); // minimum number of primary points
    m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJTYPE]        = QVariant((int)QGenerator::LinearWithKink);
	m_qmParamDefaults[SETTINGS_KEY_PROP_TAB]            = QVariant(0);
	m_qmParamDefaults[SETTINGS_KEY_GEOMETRY]            = QVariant(QSerial(QRect(200,200,200,200)).toBase64());
	m_qmParamDefaults[SETTINGS_KEY_TIMESLICE]           = QVariant(10);
	m_qmParamDefaults[SETTINGS_KEY_TICKIDX]             = QVariant(0);

    QStringList qslNames;
	qslNames 
		<< QINDICATOR_LEGEND_PRIMARY 
		<< QINDICATOR_LEGEND_SOURCE 
		<< QINDICATOR_LEGEND_SOURCE_ALARM 
        << QINDICATOR_LEGEND_FILTER
        << QINDICATOR_LEGEND_CLUSTER;
    QList<QColor> qlColors;
	QList<int> qlSymbSizes;
	QByteArray baSymbSizes;
	QDataStream dsSymbSizes(&baSymbSizes,QIODevice::ReadWrite);
	for (int i=0; i<qslNames.count(); i++) {
		qlColors << QColor(Qt::red);
		QString qsKey=qslNames.at(i);
		qsKey.replace(' ','_');
	    m_qmParamDefaults[qsKey] = QVariant(QSerial(qlColors.at(i)).toBase64());
		dsSymbSizes << DEFAULT_SYMBOL_SIZE;
		qlSymbSizes << DEFAULT_SYMBOL_SIZE;
	}
	m_qmParamDefaults[SETTINGS_KEY_SYMBSIZES]           = QVariant(baSymbSizes.toBase64());
	QIndicator::m_lsLegend.m_iNumColors=qslNames.count();
	QIndicator::m_lsLegend.m_qlSettingNames=qslNames;
	QIndicator::m_lsLegend.m_qlColors=qlColors;
	QIndicator::m_lsLegend.m_qlSizes=qlSymbSizes;
	m_qmParamDefaults[SETTINGS_KEY_IND_GEOMETRY]        = QVariant(QSerial(QRect(200,200,200,200)).toBase64());
	m_qmParamDefaults[SETTINGS_KEY_IND_SCALE]           = QVariant(1.0e-3);
	QIndicator::m_dDefaultScale = m_qmParamDefaults[SETTINGS_KEY_IND_SCALE].toDouble();
	m_qmParamDefaults[SETTINGS_KEY_IND_VIEWX0]          = QVariant(0.0e0);
	m_qmParamDefaults[SETTINGS_KEY_IND_VIEWY0]          = QVariant(0.0e0);
	m_qmParamDefaults[SETTINGS_KEY_IND_GRIDSTEP]        = QVariant(50.0e0);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStopper::QStopper()
: QSplashScreen(QPixmap(), Qt::WindowStaysOnTopHint)
, m_pmHarddisk(QPixmap(":/Resources/hdd_exch.png")) {
	setPixmap(m_pmHarddisk);
	resize(m_pmHarddisk.width(),m_pmHarddisk.height());
	move(x()-m_pmHarddisk.width()/2,y()-m_pmHarddisk.height()/2);
	show();
	repaint();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QStopper::drawContents (QPainter *painter) {
    // qDebug() << "Inside draw contents";
	// painter->drawPixmap(0,0,m_pmHarddisk);
    painter->setBrush(QBrush(Qt::blue));
    painter->setPen(Qt::blue);
    painter->setFont(QFont("Arial", 16));
	painter->drawText(QRect(0,m_pmHarddisk.height()-30,m_pmHarddisk.width(),30),Qt::AlignCenter,"Connecting to DB ...");
//	painter->fillRect(10,10,100,100,Qt::SolidPattern);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStopper::~QStopper() {
}
