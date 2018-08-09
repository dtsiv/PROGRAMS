#include "qproppages.h"
#include <QPixmap>
#include "qtracefilter.h"
#include "qserial.h"
#include "qindicator.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColorSelectionButton::QColorSelectionButton(const QColor &c, QWidget *parent /*=0*/) : 
  QPushButton(parent)
, m_qcColorSelection(c) 
, m_pcdColorDlg(NULL)  {
	setContentsMargins(2,2,2,2);
	QObject::connect(this,SIGNAL(clicked()),SLOT(onClicked()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColorSelectionButton::~QColorSelectionButton() {
	if (m_pcdColorDlg) delete m_pcdColorDlg;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColor& QColorSelectionButton::getSelection() {
	return m_qcColorSelection;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::paintEvent(QPaintEvent *pe) {
	this->QPushButton::paintEvent(pe);
	QPainter painter(this);
	painter.fillRect(contentsRect(), m_qcColorSelection);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::onClicked() {
	m_pcdColorDlg = new QColorDialog (m_qcColorSelection);
	m_pcdColorDlg->setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));
    m_pcdColorDlg->setOption(QColorDialog::ShowAlphaChannel);
	m_pcdColorDlg->open(this, SLOT(onColorSelected(QColor)));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::onColorSelected(const QColor &color) {
	m_qcColorSelection=color;
	update();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPropPages::QPropPages(QTraceFilter *pOwner
	, QMap<QString,QVariant> &qmParamDefaults
	, QWidget *parent /* =0 */)
 : QDialog(parent,Qt::Dialog)
 , m_pOwner(pOwner) 
 , m_qmParamDefaults(qmParamDefaults) 
 , m_pbgDBEngine(NULL) {
	ui.setupUi(this);
	ui.tabWidget->setStyleSheet(
      "QTabBar::tab:selected { background: lightgray; } ");

	m_qslPages << PROPPAGE_DB << PROPPAGE_MAINCTRL << PROPPAGE_KALMAN 
		       << PROPPAGE_IMITATOR << PROPPAGE_INDICATOR;
	for (int i = 0; i < m_qslPages.size(); ++i) addTab(i);
	if (m_pOwner->m_iActiveTab<m_qslPages.size()) ui.tabWidget->setCurrentIndex(m_pOwner->m_iActiveTab);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPropPages::~QPropPages() {
	m_pOwner->m_iActiveTab = ui.tabWidget->currentIndex();
	if (m_pbgDBEngine) delete m_pbgDBEngine;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::addTab(int iIdx) {
	QWidget *pWidget=new QWidget;
	pWidget->setAutoFillBackground(true);
	// pWidget->setStyleSheet("QWidget { background: gray; } ");
	QVBoxLayout *pLayout=new QVBoxLayout;
	if (iIdx==m_qslPages.indexOf(PROPPAGE_DB)){
		m_pbgDBEngine=new QButtonGroup;
		m_prbDBEnginePostgres=new QRadioButton("PostgreQSL: DB connection string");
		m_pbgDBEngine->addButton(m_prbDBEnginePostgres);
		pLayout->addWidget(m_prbDBEnginePostgres);
		m_pleDbConn=new QLineEdit(m_pOwner->m_qsPgConnStr);
		pLayout->addWidget(m_pleDbConn);
		m_prbDBEngineSqlite=new QRadioButton("SQLite: DB file");
		m_pbgDBEngine->addButton(m_prbDBEngineSqlite);
		pLayout->addWidget(m_prbDBEngineSqlite);
		QHBoxLayout *pHBoxLayout01=new QHBoxLayout;
		QString qsSqliteDbFile=m_pOwner->m_qsSqliteFilePath;
		QFileInfo fiSqliteDbFile(qsSqliteDbFile);
		if (fiSqliteDbFile.exists() && fiSqliteDbFile.isFile()) {
			qsSqliteDbFile=QDir::current().relativeFilePath(qsSqliteDbFile);
		}
		m_pleSqliteDbFile=new QLineEdit(qsSqliteDbFile);
		pHBoxLayout01->addWidget(m_pleSqliteDbFile);
		QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
		QObject::connect(ppbChoose,SIGNAL(clicked()),SLOT(onSQLiteFileChoose()));
		pHBoxLayout01->addWidget(ppbChoose);
		pLayout->addLayout(pHBoxLayout01);
		pLayout->addStretch();
		if (m_pOwner->m_dbeEngineSelected==QPoiModel::DB_Engine_Postgres) {
            m_prbDBEnginePostgres->setChecked(true);
		}
		else if (m_pOwner->m_dbeEngineSelected==QPoiModel::DB_Engine_Sqlite) {
            m_prbDBEngineSqlite->setChecked(true);
		}
		else {
            m_prbDBEnginePostgres->setChecked(false);
            m_prbDBEngineSqlite->setChecked(false);
		}
	}
	else if (iIdx==m_qslPages.indexOf(PROPPAGE_MAINCTRL)) {
		QHBoxLayout *pHLayout=new QHBoxLayout;
		pHLayout->addWidget(new QLabel("cfg file:"));
		QGraphicsView *pView=new QGraphicsView(&m_scene);
		pView->scale(5,5);
		QHBoxLayout *pHLayout1=new QHBoxLayout;
		pHLayout1->addWidget(pView);
		m_plbAvrBL=new QLabel("Location (deg)");
		pHLayout1->addWidget(m_plbAvrBL);
		pHLayout1->addStretch();

		// Mainctrl file
		QString qsMainctrlCfg=m_pOwner->m_qsMainctrlCfg;
		QDir qdCur=QDir::current();
		if (qdCur.exists(qsMainctrlCfg)) {
			m_pOwner->m_qsMainctrlCfg=qdCur.absoluteFilePath(qsMainctrlCfg);
			m_pleMainCtrl=new QLineEdit(qdCur.relativeFilePath(qsMainctrlCfg));
			showPosts(m_pOwner->m_qsMainctrlCfg);
		}
		else {
			m_pleMainCtrl=new QLineEdit(qsMainctrlCfg);
		}
		QObject::connect(m_pleMainCtrl,SIGNAL(textEdited(QString)),
			SLOT(onMainctrlChanged(QString)));
		pHLayout->addWidget(m_pleMainCtrl);
		QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
		QObject::connect(ppbChoose,SIGNAL(clicked()),
			SLOT(onMainctrlChoose()));
		pHLayout->addWidget(ppbChoose);
		pLayout->addLayout(pHLayout);
		pLayout->addLayout(pHLayout1);
		//pLayout->addStretch();
	}
	else if (iIdx==m_qslPages.indexOf(PROPPAGE_KALMAN)) { // Kalman filter parameters
		QGridLayout *pGridLayout=new QGridLayout;
		
		// system noise sigma (m/s)
		pGridLayout->addWidget(new QLabel("System noise s2(m2/s3)"),0,0);
		m_pleKalmanSigmaS=new QLineEdit(QString::number(QVoiProcessor::m_dFltSigmaS2,'e',1));
		m_pleKalmanSigmaS->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanSigmaS->setValidator(new QDoubleValidator(0.0,
             999.0, 1, m_pleKalmanSigmaS));
		pGridLayout->addWidget(m_pleKalmanSigmaS,0,1);

		// meas noise sigma (m)
		pGridLayout->addWidget(new QLabel("Measurement noise (m)"),0,3);
		m_pleKalmanSigmaM=new QLineEdit(QString::number(QVoiProcessor::m_dFltSigmaM,'f',3));
		m_pleKalmanSigmaM->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanSigmaM->setValidator(new QDoubleValidator(0.0,
             999.0, 4, m_pleKalmanSigmaM));
		pGridLayout->addWidget(m_pleKalmanSigmaM,0,4);

		// Height
		pGridLayout->addWidget(new QLabel("Target height (km)"),1,0);
		m_pleKalmanHeight=new QLineEdit(QString::number(QVoiProcessor::m_dFltHeight,'f',1));
		m_pleKalmanHeight->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanHeight->setValidator(new QDoubleValidator(0.0,
             30.0, 1, m_pleKalmanHeight));
		pGridLayout->addWidget(m_pleKalmanHeight,1,1);

		// Strob min size km
		pGridLayout->addWidget(new QLabel("Min strob (km)"),2,0);
		m_pleKalmanMinStrob=new QLineEdit(QString::number(QVoiProcessor::m_dStrobSpreadMin,'e',1));
		m_pleKalmanMinStrob->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanMinStrob->setValidator(new QDoubleValidator(0.0,
             1.0e3, 1, m_pleKalmanMinStrob));
		pGridLayout->addWidget(m_pleKalmanMinStrob,2,1);

		// Strob spread m/s
		pGridLayout->addWidget(new QLabel("Strob spread (m/s)"),3,0);
		m_pleKalmanStrobSpread=new QLineEdit(QString::number(QVoiProcessor::m_dStrobSpreadSpeed,'e',1));
		m_pleKalmanStrobSpread->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanStrobSpread->setValidator(new QDoubleValidator(0.0,
             1.0e3, 1, m_pleKalmanStrobSpread));
		pGridLayout->addWidget(m_pleKalmanStrobSpread,3,1);

		// Chi2 probability threshold
		pGridLayout->addWidget(new QLabel("Chi2 probability"),1,3);
		m_pcbKalmanChi2Prob=new QComboBox;
		m_pcbKalmanChi2Prob->setMaximumWidth(PROP_LINE_WIDTH);
		int iMax=sizeof(QVoiProcessor::m_dChi2QuantProb)/sizeof(QVoiProcessor::m_dChi2QuantProb[0]);
		for (int i=0; i<iMax; i++) {
			double dProb=QVoiProcessor::m_dChi2QuantProb[i];
            m_pcbKalmanChi2Prob->addItem(QString("%1").arg(dProb,0,'f',3));
		}
		m_pcbKalmanChi2Prob->setCurrentIndex(qBound(0,QVoiProcessor::m_iChi2Prob,iMax));
		pGridLayout->addWidget(m_pcbKalmanChi2Prob,1,4);

		// Use matrix relaxation
		pGridLayout->addWidget(new QLabel("Covariations P"),2,3);
		m_pcbKalmanUseRelax=new QCheckBox("use relaxation");
		m_pcbKalmanUseRelax->setChecked(QVoiProcessor::m_bUseMatRelax);
		pGridLayout->addWidget(m_pcbKalmanUseRelax,2,4);

		// Matrix relaxation time (sec)
		pGridLayout->addWidget(new QLabel("P relaxation (s)"),3,3);
		m_pleKalmanMatRelaxTime=new QLineEdit(QString::number(QVoiProcessor::m_dMatRelaxTime,'e',1));
		m_pleKalmanMatRelaxTime->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleKalmanMatRelaxTime->setValidator(new QDoubleValidator(0.0,
             1.0e5, 1, m_pleKalmanMatRelaxTime));
		pGridLayout->addWidget(m_pleKalmanMatRelaxTime,3,4);

		// stretch
		pGridLayout->setColumnStretch(2,100);
		pGridLayout->setColumnStretch(5,100);
		pLayout->addLayout(pGridLayout);
		pLayout->addStretch();
	}
	else if (iIdx==m_qslPages.indexOf(PROPPAGE_IMITATOR)) { // POITE generator parameters
		QGridLayout *pGridLayout01=new QGridLayout;

		// meas noise sigma (m)
		pGridLayout01->addWidget(new QLabel("Measurement noise (m)"),0,0);
		m_pleGenSigmaM=new QLineEdit(QString::number(QGenerator::m_dGenSigmaM,'f',3));
		m_pleGenSigmaM->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenSigmaM->setValidator(new QDoubleValidator(0.0,
             999.0, 4, m_pleGenSigmaM));
		pGridLayout01->addWidget(m_pleGenSigmaM,0,1);

		// meas delay
		pGridLayout01->addWidget(new QLabel("Measurement delay (s)"),0,3);
		m_pleGenDelay=new QLineEdit(QString::number(QGenerator::m_dGenDelay,'f',1));
		m_pleGenDelay->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenDelay->setValidator(new QDoubleValidator(0.0,
             999.0, 1, m_pleGenDelay));
		pGridLayout01->addWidget(m_pleGenDelay,0,4);

		// Initial X0
		pGridLayout01->addWidget(new QLabel("Init X (km)"),1,0);
		m_pleGenX0=new QLineEdit(QString::number(QGenerator::m_dGenX0,'f',1));
		m_pleGenX0->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenX0->setValidator(new QDoubleValidator(-999.0,
             999.0, 1, m_pleGenX0));
		pGridLayout01->addWidget(m_pleGenX0,1,1);

		// Initial Y0
		pGridLayout01->addWidget(new QLabel("Init Y (km)"),1,3);
		m_pleGenY0=new QLineEdit(QString::number(QGenerator::m_dGenY0,'f',1));
		m_pleGenY0->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenY0->setValidator(new QDoubleValidator(-999.0,
             999.0, 1, m_pleGenY0));
		pGridLayout01->addWidget(m_pleGenY0,1,4);

		// Initial velocity magnitude
		pGridLayout01->addWidget(new QLabel("Init V (m/s)"),2,0);
		m_pleGenInitV=new QLineEdit(QString::number(QGenerator::m_dGenV0,'f',1));
		m_pleGenInitV->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenInitV->setValidator(new QDoubleValidator(0.0,
             999.0, 1, m_pleGenInitV));
		pGridLayout01->addWidget(m_pleGenInitV,2,1);

		// Initial velocity azimuth
		pGridLayout01->addWidget(new QLabel("V azimuth (deg)"),2,3);
		m_pleGenInitAz=new QLineEdit(QString::number(QGenerator::m_dGenAz,'f',1));
		m_pleGenInitAz->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenInitAz->setValidator(new QDoubleValidator(-360.0,
             360.0, 1, m_pleGenInitAz));
		pGridLayout01->addWidget(m_pleGenInitAz,2,4);

		// Trajectory curvature radius
		pGridLayout01->addWidget(new QLabel("Curvature (km)"),3,0);
		m_pleGenTrajR=new QLineEdit(QString::number(QGenerator::m_dGenTrajR,'f',1));
		m_pleGenTrajR->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenTrajR->setValidator(new QDoubleValidator(0.0,
             1.0e5, 1, m_pleGenTrajR));
		pGridLayout01->addWidget(m_pleGenTrajR,3,1);

		// Trajectory clock wise
		pGridLayout01->addWidget(new QLabel("Rotation direction"),3,3);
		m_pcbGenTrajRotCW=new QCheckBox("clockwise");
		m_pcbGenTrajRotCW->setChecked(QGenerator::m_bGenTrajCW);
		pGridLayout01->addWidget(m_pcbGenTrajRotCW,3,4);

		// Height
		pGridLayout01->addWidget(new QLabel("Target height (km)"),4,0);
		m_pleGenHeight=new QLineEdit(QString::number(QGenerator::m_dGenHeight,'f',1));
		m_pleGenHeight->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenHeight->setValidator(new QDoubleValidator(0.0,
             30.0, 1, m_pleGenHeight));
		pGridLayout01->addWidget(m_pleGenHeight,4,1);

        // Kinik time s
		pGridLayout01->addWidget(new QLabel("Trajectory duration (min)"),4,3);
		m_pleTrajDuration=new QLineEdit(QString::number(QVoiProcessor::m_dTrajDuration,'f',1));
		m_pleTrajDuration->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleTrajDuration->setValidator(new QDoubleValidator(0.0,
             10.0*60, 1, m_pleTrajDuration));
		pGridLayout01->addWidget(m_pleTrajDuration,4,4);

        // Kinik time s
		pGridLayout01->addWidget(new QLabel("Kink time (min)"),5,0);
		m_pleGenKinkTime=new QLineEdit(QString::number(QGenerator::m_dGenKinkTime,'f',1));
		m_pleGenKinkTime->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenKinkTime->setValidator(new QDoubleValidator(0.0,
             10.0*3600, 1, m_pleGenKinkTime));
		pGridLayout01->addWidget(m_pleGenKinkTime,5,1);

		// Kinik angle deg
		pGridLayout01->addWidget(new QLabel("Kink angle (deg)"),5,3);
		m_pleGenKinkAngle=new QLineEdit(QString::number(QGenerator::m_dGenKinkAngle,'f',1));
		m_pleGenKinkAngle->setMaximumWidth(PROP_LINE_WIDTH);
		m_pleGenKinkAngle->setValidator(new QDoubleValidator(-360.0,
             360.0, 1, m_pleGenKinkAngle));
		pGridLayout01->addWidget(m_pleGenKinkAngle,5,4);

		pGridLayout01->setColumnStretch(5,100);
		pGridLayout01->setColumnStretch(2,100);
		pLayout->addLayout(pGridLayout01);
		pLayout->addStretch();
	}
	else if (iIdx==m_qslPages.indexOf(PROPPAGE_INDICATOR)) { // indicator parameters
		QGridLayout *pGridLayout=new QGridLayout;

		// Use Tolerance
		pGridLayout->addWidget(new QLabel("2D hyperbolic eqn"),0,0);
		m_pcbKalmanUseTol=new QCheckBox("use tolerance");
		m_pcbKalmanUseTol->setChecked(QPsVoi::m_UseTolerance);
		pGridLayout->addWidget(m_pcbKalmanUseTol,0,1);
		
		// Tolerance
		pGridLayout->addWidget(new QLabel("Tolerance"),1,0);
        m_pleKalmanTolerance=new QLineEdit(QString::number(QPsVoi::m_Tolerance,'e',1));
		m_pleKalmanTolerance->setMaximumWidth(PROP_LINE_WIDTH);
		QDoubleValidator *pdvTolerance=new QDoubleValidator(-10.0e0,
             10.0e0, 1, m_pleKalmanTolerance);
		pdvTolerance->setNotation(QDoubleValidator::ScientificNotation);
		m_pleKalmanTolerance->setValidator(pdvTolerance);
		pGridLayout->addWidget(m_pleKalmanTolerance,1,1);
		
		// Scale
		pGridLayout->addWidget(new QLabel("View scale (pix/m)"),2,0);
        m_pleIndViewScale=new QLineEdit(QString::number(QIndicator::m_dScale,'e',1));
		m_pleIndViewScale->setMaximumWidth(PROP_LINE_WIDTH);
		QDoubleValidator *pdvScale=new QDoubleValidator(QIndicator::m_dScaleMin,
             QIndicator::m_dScaleMax, 1, m_pleIndViewScale);
		pdvTolerance->setNotation(QDoubleValidator::ScientificNotation);
		m_pleIndViewScale->setValidator(pdvScale);
		pGridLayout->addWidget(m_pleIndViewScale,2,1);
		
		// View X0
		pGridLayout->addWidget(new QLabel("View origin X (km)"),3,0);
        m_pleIndViewX0=new QLineEdit(QString::number(QIndicator::m_dViewX0,'f',1));
		m_pleIndViewX0->setMaximumWidth(PROP_LINE_WIDTH);
		QDoubleValidator *pdvViewX0=new QDoubleValidator(-1.0e3,
             1.0e3, 1, m_pleIndViewX0);
		m_pleIndViewX0->setValidator(pdvViewX0);
		pGridLayout->addWidget(m_pleIndViewX0,3,1);
		
		// View Y0
		pGridLayout->addWidget(new QLabel("View origin Y (km)"),4,0);
        m_pleIndViewY0=new QLineEdit(QString::number(QIndicator::m_dViewY0,'f',1));
		m_pleIndViewY0->setMaximumWidth(PROP_LINE_WIDTH);
		QDoubleValidator *pdvViewY0=new QDoubleValidator(-1.0e3,
             1.0e3, 1, m_pleIndViewY0);
		m_pleIndViewY0->setValidator(pdvViewY0);
		pGridLayout->addWidget(m_pleIndViewY0,4,1);
		
		// grid step
		pGridLayout->addWidget(new QLabel("Grid step (km)"),5,0);
        m_pleIndGridStep=new QLineEdit(QString::number(QIndicator::m_dGridStep,'f',1));
		m_pleIndGridStep->setMaximumWidth(PROP_LINE_WIDTH);
		QDoubleValidator *pdvGridStep=new QDoubleValidator(1.0e0,
             1.0e3, 1, m_pleIndGridStep);
		m_pleIndGridStep->setValidator(pdvGridStep);
		pGridLayout->addWidget(m_pleIndGridStep,5,1);

		// Legend colors
		QStringList qslNames=QIndicator::m_lsLegend.m_qlSettingNames;
		QList<int> qlSizes=QIndicator::m_lsLegend.m_qlSizes;
		QList<QColor> qlColors=QIndicator::m_lsLegend.m_qlColors;
		int iNumColors=QIndicator::m_lsLegend.m_iNumColors;
		for (int i=0; i<iNumColors; i++) {
			QColorSelectionButton *pcsbColorSelection=new QColorSelectionButton(qlColors.at(i));
			QSpinBox *psbSize=new QSpinBox;
			psbSize->setValue(qlSizes.at(i)); 
			psbSize->setMaximumWidth(PROP_LINE_WIDTH); 
			psbSize->setMinimum(0); 
			psbSize->setMaximum(100);
			QString qsKey=qslNames.at(i);
			qsKey.replace(' ','_');
			m_qmIndLegendColors.insert(qsKey,pcsbColorSelection);
            m_qmIndSymbSizes.insert(qsKey,psbSize);
            pGridLayout->addWidget(new QLabel(qslNames.at(i)),i,3);
		    pGridLayout->addWidget(pcsbColorSelection,i,4);
			pGridLayout->addWidget(psbSize,i,5);
		}

		pGridLayout->setColumnStretch(5,100);
		pGridLayout->setColumnStretch(2,100);
		pLayout->addLayout(pGridLayout);
		pLayout->addStretch();
	}
	pWidget->setLayout(pLayout);
	ui.tabWidget->addTab(pWidget,m_qslPages.at(iIdx));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::onMainctrlChanged(QString qsNewText) {
	m_scene.clear();
	QDir qdCur=QDir::current();
	if (!qdCur.exists(qsNewText)) return;
	QString qsMainctrlCfg=qdCur.absoluteFilePath(qsNewText);
	QFileInfo fi(qsMainctrlCfg);
	if (fi.isFile() && fi.isReadable())	{
        m_pOwner->m_qsMainctrlCfg=qsMainctrlCfg;
        showPosts(m_pOwner->m_qsMainctrlCfg);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::onSQLiteFileChoose() {
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("SQLite db file (*.db *.sqlite *.sqlite3)"));
	dialog.setDirectory(QDir::current());
	if (dialog.exec()) {
		QStringList qsSelection=dialog.selectedFiles();
		if (qsSelection.size() != 1) return;
		QString qs=qsSelection.at(0);
		QDir qdCur=QDir::current();
		if (!qdCur.exists(qs)) return;
		qs=qdCur.absoluteFilePath(qs);
		QFileInfo fi(qs);
		if (fi.isFile() && fi.isReadable())	{
			QString qsSqlitePath=qdCur.absoluteFilePath(qs);
			m_pleSqliteDbFile->setText(qdCur.relativeFilePath(qsSqlitePath));
		}
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::onMainctrlChoose() {
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("MainCtrl files (*.cfg)"));
	dialog.setDirectory(QDir::current());
	if (dialog.exec()) {
		QStringList qsSelection=dialog.selectedFiles();
		if (qsSelection.size() != 1) return;
		QString qs=qsSelection.at(0);
		QDir qdCur=QDir::current();
		if (!qdCur.exists(qs)) return;
		qs=qdCur.absoluteFilePath(qs);
		QFileInfo fi(qs);
		if (fi.isFile() && fi.isReadable())	{
			m_pOwner->m_qsMainctrlCfg=qdCur.absoluteFilePath(qs);
			m_pleMainCtrl->setText(qdCur.relativeFilePath(m_pOwner->m_qsMainctrlCfg));
			showPosts(m_pOwner->m_qsMainctrlCfg);
		}
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::showPosts(QString qsMainctrlCfg) {
	QFile qfMainCtrl(qsMainctrlCfg);
	m_scene.clear();
	if (qfMainCtrl.open(QIODevice::ReadOnly)) {
		PMAINCTRL m_pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
		qfMainCtrl.close();
		int i;
	    double dScale=1.0e-3;
		QGeoUtils gu;
		gu.getViewPoint(&m_blhViewPoint,m_pMainCtrl); // radians,meters
		for (i=1; i<=4; i++) {
			PGROUNDINFO pgi = &m_pMainCtrl -> p.positions[i];
			XYZ postXYZ;
			BLH postBLH;
			postBLH.dLat=pgi->blh.dLat*gu.DEG_2_RAD; postBLH.dLon=pgi->blh.dLon*gu.DEG_2_RAD;
			postBLH.dHei=pgi->blh.dHei;
			gu.toTopocentric(&m_blhViewPoint,&postBLH,&postXYZ); // radians,meters
			m_scene.addText(QString::number(i))->setPos(postXYZ.dX*dScale,-postXYZ.dY*dScale);
		}
		m_plbAvrBL->setMargin(20);
		m_plbAvrBL->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
		m_plbAvrBL->setText(QString("View point (deg)\n(%1, %2)")
			  .arg(m_blhViewPoint.dLat*gu.RAD_2_DEG,0,'f',1).arg(m_blhViewPoint.dLon*gu.RAD_2_DEG,0,'f',1));
	}
	else {
	    m_scene.addText("Open failed");
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPoiModel::POI_DB_ENGINES QPropPages::getSelectedDBEngine() {
	if (m_prbDBEngineSqlite->isChecked()) return QPoiModel::DB_Engine_Sqlite;
	else if (m_prbDBEnginePostgres->isChecked()) return QPoiModel::DB_Engine_Postgres;
	else return QPoiModel::DB_Engine_Undefined;
}
