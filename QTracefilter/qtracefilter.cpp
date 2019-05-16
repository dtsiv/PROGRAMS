#include "QTraceFilter.h"
#include "qserial.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>

#include "qvoiprocessor.h"
#include "qkalmanfilter.h"

QTraceFilter::QTraceFilter(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags) 
	, m_pSettings(NULL)
	, m_pPoiModel(NULL)
	, m_pIndicator(NULL)
	, exitAct(NULL)
	, aboutAct(NULL)
	, updateAct(NULL)
	, settingsAct(NULL)
	, fileMenu(NULL)
	, helpMenu(NULL)
	, lbStatusMsg(NULL)
	, lbStatusArea(NULL)
	, m_qsMainctrlCfg("mainctrl.cfg")
	, MAX_TICK(199)
	, TIME_LBL_FMT("dd.MM.yyyy hh:mm:ss.zzz") 
	, m_dbeEngineSelected(QPoiModel::DB_Engine_Undefined) 
    , m_bSetup(false) 
    , m_pbgTrajectoryType(NULL) 
	, m_pbgSkipPost(NULL)
	, m_pgbImit(NULL)
	, m_pgbPoiteDb(NULL) {

	arrangeControls();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QTraceFilter::~QTraceFilter() {
	if (m_pIndicator) {
		delete m_pIndicator;
		m_pIndicator=NULL;
	}
    writeSettings();
	if (m_pPoiModel) delete m_pPoiModel;
	if (m_pSettings) delete m_pSettings;
	if (m_pbgTrajectoryType) delete m_pbgTrajectoryType;
	if (m_pbgSkipPost) delete m_pbgSkipPost;
	if (m_pgbImit) delete m_pgbImit;
	if (m_pgbPoiteDb) delete m_pgbPoiteDb;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::closeEvent(QCloseEvent *event) {
	if (m_pIndicator) {
		delete m_pIndicator;
		m_pIndicator=NULL;
	}
    event->accept();
 }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::refreshTimeSlider() {
		// DB connection
	bool bTimeQueryOk = m_pPoiModel->getMinMaxTime(m_tFrom,m_tTo);
	if (!bTimeQueryOk) {
		qDebug() << "getMinMaxTime(m_tFrom,m_tTo) failed!";
		// throw RmoException("QTraceFilter error: getMinMaxTime!");
		return;
	}
	m_uMsecsTot = m_tTo-m_tFrom;
	m_currTimeMSecs = m_tFrom;
    onSliceSelected(comboTimeSlice->currentIndex());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onValueChanged(int iVal) {
	if (!m_uMaxTick) {
		qDebug() << "QTraceFilter error: m_uMaxTick==0!";
		// throw RmoException("QTraceFilter error: m_uMaxTick==0!");
		return;
	}
	m_uTickIdx=iVal;
	m_currTimeMSecs = m_tFrom + iVal * m_uMsecsTot / (m_uMaxTick+1);
	QDateTime tCur=QDateTime::fromMSecsSinceEpoch(m_currTimeMSecs);
	QDateTime tUTC=tCur.toUTC();
	tUTC.setTimeSpec(Qt::LocalTime);
	int iOffsTZ = tUTC.secsTo(tCur)/3600;
	QString qsOffsTZ=(iOffsTZ>0)?
		 QString("+%1").arg(iOffsTZ,2,10,QLatin1Char('0'))
		:QString("%1").arg(iOffsTZ,2,10,QLatin1Char('0'));
	QLocale locale(QLocale::English,QLocale::UnitedStates);
    QString qsDate=locale.toString(tCur,"ddd d MMM yyyy");
	lblTime->setText(qsDate+" "+tCur.toString("hh:mm:ss")+" - "
					   +tCur.addMSecs(m_uMsecsTot/(m_uMaxTick+1)).toString("hh:mm:ss")
					   +" GMT"+qsOffsTZ);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onSliceSelected(int iIndex) {
	if (iIndex<0 || iIndex>m_qlTimeSlices.count()-1) throw RmoException("onSliceSelected(): wrong iIndex");
    if (m_qlTimeSlices.count()<1) throw RmoException("m_qlTimeSlices: no items");
	int iMinSlice,iMaxSlice;
	iMaxSlice=iMinSlice=m_qlTimeSlices.at(0);
	for (int i=0; i<m_qlTimeSlices.count()-1; i++) {
		int iCurSlice=m_qlTimeSlices.at(i);
		iMinSlice=(iMinSlice>iCurSlice)?iCurSlice:iMinSlice;
		iMaxSlice=(iMaxSlice<iCurSlice)?iCurSlice:iMaxSlice;
	}
	if (iMinSlice*60000>m_uMsecsTot) 
		throw RmoException("onSliceSelected(): Time interval too small");
	if ((quint64)iMaxSlice*(MAX_TICK+1)*60000<m_uMsecsTot) 
		throw RmoException("onSliceSelected(): Time interval too big");
	quint32 uMinu = m_qlTimeSlices.at(iIndex);
    quint32 uMaxTick = qRound((double)m_uMsecsTot/uMinu/60000-0.5e0);
	int iCurr=-1;
	if (uMaxTick < 1 || uMaxTick > MAX_TICK) {
		for (int i=m_qlTimeSlices.count()-1; i>=0; i--) {
			uMinu = m_qlTimeSlices.at(i);
			uMaxTick = qRound((double)m_uMsecsTot/uMinu/60000-0.5e0);
			if (uMaxTick >= 1 && uMaxTick <= MAX_TICK) {
				iCurr=i; break;
			}
		}
		if (iCurr<0) throw RmoException("onSliceSelected(): slice autoselection failed");
	}
	m_uTimeSlice = uMinu;
	m_uMaxTick = uMaxTick;
	double dOffsetMinutes=(m_currTimeMSecs - m_tFrom) / 60.0e3;
	quint32 uCurrTick = qBound((quint32)0,(quint32)qRound(dOffsetMinutes / m_uTimeSlice),m_uMaxTick) ;
	horizontalSlider->setValue(0);
    horizontalSlider->setMaximum(m_uMaxTick);
	horizontalSlider->setValue(uCurrTick);
	onValueChanged(uCurrTick);
    m_uTickIdx = uCurrTick;
	if (iCurr >= 0) comboTimeSlice->setCurrentIndex(iCurr);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onSetup() {
	bool bOk;

	m_bSetup=true;
	QPropPages propDlg(this,m_qmParamDefaults,this);
	bool bDlgRes=propDlg.exec();
	m_bSetup=false;

	if (!bDlgRes) return;
	// DB engines
	QString qsSqliteFilePath=propDlg.m_pleSqliteDbFile->text();
    QFileInfo fiSqliteDb(qsSqliteFilePath);
	QPoiModel::POI_DB_ENGINES dbeEngineSelected = propDlg.getSelectedDBEngine();
	QString qsPgConnStr=propDlg.m_pleDbConn->text();
	// check if connection state changed
	if (m_dbeEngineSelected!=dbeEngineSelected 
	     || (dbeEngineSelected==QPoiModel::DB_Engine_Postgres && m_qsPgConnStr!=qsPgConnStr)
         || (dbeEngineSelected==QPoiModel::DB_Engine_Sqlite && m_qsSqliteFilePath!=qsSqliteFilePath)) {
		m_pPoiModel->releaseDataSource(); // make sure old db is not locked
		displayConnStatus(QPoiModel::DB_Engine_Undefined);
	}
	bool bUseGen = prbGen->isChecked(); // use generator flag: if set, no data source used
	if (bUseGen) { // no data source used
		m_dbeEngineSelected=dbeEngineSelected; // just blindly save entered values, do not connect
		m_qsPgConnStr=qsPgConnStr;
        if (fiSqliteDb.isFile() && fiSqliteDb.isReadable()) {
            QDir qdSqliteDb=fiSqliteDb.absoluteDir();
            QDir qdCur=QDir::current();
            if (qdSqliteDb.rootPath()==qdCur.rootPath()) {
                m_qsSqliteFilePath=qdCur.relativeFilePath(fiSqliteDb.absoluteFilePath());
            } else {
                m_qsSqliteFilePath=fiSqliteDb.absoluteFilePath();
            }
		}
	}
	bool bDataSourceChanged=false; // flag for SUCCESSFUL change of ACTIVE data source
	if (!bUseGen) { // data base as data source
		if (dbeEngineSelected == QPoiModel::DB_Engine_Postgres) {
            if (m_pPoiModel->initPostgresDB(qsPgConnStr)) {
				if (m_dbeEngineSelected!=dbeEngineSelected || m_qsPgConnStr!=qsPgConnStr) {
					bDataSourceChanged=true;
				}
				m_dbeEngineSelected=QPoiModel::DB_Engine_Postgres;
				m_qsPgConnStr=qsPgConnStr;
				displayConnStatus(QPoiModel::DB_Engine_Postgres);
			}
		}
		else if (dbeEngineSelected == QPoiModel::DB_Engine_Sqlite) {
            if (fiSqliteDb.isFile() && fiSqliteDb.isReadable()) {
                QFile qfSqliteDb(fiSqliteDb.absoluteFilePath());
                if (m_pPoiModel->initSqliteDB(qfSqliteDb)) {
					if (m_dbeEngineSelected!=dbeEngineSelected || m_qsSqliteFilePath!=fiSqliteDb.absoluteFilePath()) {
						bDataSourceChanged=true;
					}
					m_dbeEngineSelected=QPoiModel::DB_Engine_Sqlite;
                    QDir qdSqliteDb=fiSqliteDb.absoluteDir();
                    QDir qdCur=QDir::current();
                    if (qdSqliteDb.rootPath()==qdCur.rootPath()) { // same Windows drives
                        m_qsSqliteFilePath=qdCur.relativeFilePath(fiSqliteDb.absoluteFilePath());
                    }
                    else { // Different Windows drive
                        m_qsSqliteFilePath=fiSqliteDb.absoluteFilePath();
                    }
					displayConnStatus(QPoiModel::DB_Engine_Sqlite);
				}
			}
		}
	}
    // Mainctrl file
    QString qsMainCtrl=propDlg.m_pleMainCtrl->text();
    QFileInfo fiMainCtrl(qsMainCtrl);
    if (fiMainCtrl.isFile() && fiMainCtrl.isReadable()) {
        QDir qdMainCtrl=fiMainCtrl.absoluteDir();
        QDir qdCur=QDir::current();
        if (qdMainCtrl.rootPath() == qdCur.rootPath()) { // same Windows drive
            m_qsMainctrlCfg = qdCur.relativeFilePath(fiMainCtrl.absoluteFilePath());
        }
        else { // differenr Windows drives
            m_qsMainctrlCfg = fiMainCtrl.absoluteFilePath();
        }
    }
    else { // write as is ...
        m_qsMainctrlCfg = qsMainCtrl;
    }
	// Kalman filter parameters
	QString qs;
	qs=propDlg.m_pleKalmanSigmaS->text();
    QKalmanFilter::m_dFltSigmaS2 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dFltSigmaS2 malformed " << qs;
	qs=propDlg.m_pleKalmanSigmaM->text();
    QKalmanFilter::m_dFltSigmaM = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dFltSigmaM malformed " << qs;
	qs=propDlg.m_pleKalmanHeight->text();
    QKalmanFilter::m_dFltHeight = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dFltHeight malformed " << qs;
	qs=propDlg.m_pleKalmanTolerance->text();
	QPsVoi::m_Tolerance = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QPsVoi::m_Tolerance malformed " << qs;
	QPsVoi::m_UseTolerance = (int)propDlg.m_pcbKalmanUseTol->isChecked();;
	qs=propDlg.m_pleKalmanMinStrob->text();
    QKalmanFilter::m_dStrobSpreadMin = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dStrobSpreadMin malformed " << qs;
	qs=propDlg.m_pleKalmanStrobSpread->text();
    QKalmanFilter::m_dStrobSpreadSpeed = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dStrobSpreadSpeed malformed " << qs;
    QKalmanFilter::m_iChi2Prob = propDlg.m_pcbKalmanChi2Prob->currentIndex();
	qs=propDlg.m_pleKalmanMatRelaxTime->text();
    QKalmanFilter::m_dMatRelaxTime = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QVoiProcessor::m_dMatRelaxTime malformed " << qs;
    QKalmanFilter::m_bUseMatRelax = propDlg.m_pcbKalmanUseRelax->isChecked();
    QKalmanFilter::m_bStickTrajToModeS = propDlg.m_pcbKalmanStickToModeS->isChecked();
    qs=propDlg.m_pleKalmanClusterCutoff->text();
    QKalmanFilter::m_dClusterCutoff = qs.toDouble(&bOk);
    if (!bOk) qDebug() << "QKalmanFilter::m_dClusterCutoff malformed " << qs;
    qs=propDlg.m_pleKalmanClusterMinSz->text();
    QKalmanFilter::m_iClusterMinSize = qs.toInt(&bOk);
    if (!bOk) qDebug() << "QKalmanFilter::m_iClusterMinSize malformed " << qs;
    qs=propDlg.m_pleKalmanTrajVMax->text();
    QKalmanFilter::m_dTrajMaxVelocity = qs.toDouble(&bOk);
    if (!bOk) qDebug() << "QKalmanFilter::m_dTrajMaxVelocity malformed " << qs;
    qs=propDlg.m_pleKalmanTrajTimeout->text();
    QKalmanFilter::m_dTrajTimeout = qs.toDouble(&bOk);
    if (!bOk) qDebug() << "QKalmanFilter::m_dTrajTimeout malformed " << qs;
    QKalmanFilter::m_bEstIniVelocity = propDlg.m_pcbKalmanEstIniVelo->isChecked();

	// generator parameters
	qs=propDlg.m_pleGenSigmaM->text();
	QGenerator::m_dGenSigmaM = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenSigmaM malformed " << qs;
	qs=propDlg.m_pleGenDelay->text();
	QGenerator::m_dGenDelay = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenDelay malformed " << qs;
	qs=propDlg.m_pleGenX0->text();
	QGenerator::m_dGenX0 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenX0 malformed " << qs;
	qs=propDlg.m_pleGenY0->text();
	QGenerator::m_dGenY0 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenY0 malformed " << qs;
	qs=propDlg.m_pleGenInitV->text();
	QGenerator::m_dGenV0 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenV0 malformed " << qs;
	qs=propDlg.m_pleGenInitAz->text();
	QGenerator::m_dGenAz = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenAz malformed " << qs;
	qs=propDlg.m_pleGenTrajR->text();
	QGenerator::m_dGenTrajR = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenTrajR malformed " << qs;
	qs=propDlg.m_pleGenHeight->text();
	QGenerator::m_dGenHeight = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenHeight malformed " << qs;
	QGenerator::m_bGenTrajCW = propDlg.m_pcbGenTrajRotCW->isChecked();
	qs=propDlg.m_pleGenKinkTime->text();
	QGenerator::m_dGenKinkTime = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenKinkTime malformed " << qs;
	qs=propDlg.m_pleGenKinkAngle->text();
	QGenerator::m_dGenKinkAngle = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dGenKinkAngle malformed " << qs;
    qs=propDlg.m_pleGenClusterMinSz->text();
    QGenerator::m_iClusterMinSize = qs.toInt(&bOk);
    if (!bOk) qDebug() << "QGenerator::m_iClusterMinSize malformed " << qs;
    qs=propDlg.m_pleTrajDuration->text();
	QVoiProcessor::m_dTrajDuration = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QGenerator::m_dTrajDuration malformed " << qs;
    QStringList qslNames=QIndicator::m_lsLegend.m_qlSettingNames;
	int iNumColors=QIndicator::m_lsLegend.m_iNumColors;
	for (int i=0; i<iNumColors; i++) {
		QString qsKey=qslNames.at(i);
		qsKey.replace(' ','_');
		if (propDlg.m_qmIndLegendColors.contains(qsKey)) {
			QIndicator::m_lsLegend.m_qlColors[i]=
				propDlg.m_qmIndLegendColors.value(qsKey)->getSelection();
			QIndicator::m_lsLegend.m_qlSizes[i]=
				propDlg.m_qmIndSymbSizes.value(qsKey)->value();
		}
	}
	qs=propDlg.m_pleIndViewScale->text();
	QIndicator::m_dScale = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QIndicator::m_dScale malformed " << qs;
	qs=propDlg.m_pleIndViewX0->text();
	QIndicator::m_dViewX0 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QIndicator::m_dViewX0 malformed " << qs;
	qs=propDlg.m_pleIndViewY0->text();
	QIndicator::m_dViewY0 = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QIndicator::m_dViewY0 malformed " << qs;
	qs=propDlg.m_pleIndGridStep->text();
	QIndicator::m_dGridStep = qs.toDouble(&bOk);
	if (!bOk) qDebug() << "QTraceFilter: QIndicator::m_dGridStep malformed " << qs;
	if (bDataSourceChanged) {
		refreshTimeSlider();
		onValueChanged(0);
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::closeView() {
	if (m_pIndicator) delete m_pIndicator;
	m_pIndicator=NULL;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onUpdate() {

    // note user choice like traj type and so on
    onControlToggled();

    // init QVoiProcessor
	buttonUpdate->setEnabled(false);
	closeView();
    m_pIndicator=new QIndicator(QIcon(QPixmap(":/Resources/binocle.ico")),m_pPoiModel);
	QVoiProcessor vp(this,m_pPoiModel);
    if (!vp.init(m_qsMainctrlCfg)) {
        buttonUpdate->setEnabled(true);
        throw RmoException(QString("File: ")+m_qsMainctrlCfg+" is not a valid configuration!");
    }
    QObject::connect(&vp,SIGNAL(indicatorPoint(double,double,int,TPoiT*,int)),m_pIndicator,SLOT(addPoint(double,double,int,TPoiT*,int)));
	QObject::connect(&vp,SIGNAL(indicatorUpdate()),m_pIndicator,SLOT(indicatorUpdate()));

    // posts coordinates
	QObject::connect(&vp,SIGNAL(addPost(double,double,int)),m_pIndicator,SLOT(addPost(double,double,int)));
	vp.listPosts();

    // start simulation/imitator
	if (vp.m_bUseGen) {
		quint64 uTimeTo=vp.m_dTrajDuration*60.0e3; // Imitator duration msec
        vp.start(0,uTimeTo,true);
	}
	else {
        // calculate start time and end time
        quint64 uMsecs = horizontalSlider->value() * m_uMsecsTot / (m_uMaxTick+1);
        m_tSliceFrom=m_tFrom+uMsecs;
        m_tSliceTo=m_tSliceFrom+m_uMsecsTot/(m_uMaxTick+1);

        m_pPoiModel->clearRawLists();
        if (!m_pPoiModel->readPoite(m_tSliceFrom,m_tSliceTo,QKalmanFilter::m_dFltHeight*1.0e3)) {
            throw RmoException("m_pPoiModel->readPoite failed");
            return;
        }
        vp.start(m_tSliceFrom,m_tSliceTo,false);
	}

    // show indicator window and re-enable update button
	m_pIndicator->show();
	buttonUpdate->setEnabled(true);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::onClosed() {
	if (m_pIndicator) {
		delete m_pIndicator;
		m_pIndicator=NULL;
	}
    close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::readSettings() {
	bool bOk;
	m_pSettings->beginGroup(SETTINGS_GROUP_NAME);

	// active prop page
	m_iActiveTab=m_pSettings->value(SETTINGS_KEY_PROP_TAB,
		          m_qmParamDefaults[SETTINGS_KEY_PROP_TAB]).toInt();

	// front panel geometry
	QString qsEncodedGeometry=m_pSettings->value(SETTINGS_KEY_GEOMETRY, 
		                       m_qmParamDefaults[SETTINGS_KEY_GEOMETRY]).toString();
	QRect qrGeometry = QSerial(qsEncodedGeometry).toRect(&bOk);
	if (!bOk) qrGeometry = QRect(200, 200, 200, 200);
    setGeometry(qrGeometry);

    // misc params
	m_uTickIdx = m_pSettings->value(SETTINGS_KEY_TICKIDX, 
                  m_qmParamDefaults[SETTINGS_KEY_TICKIDX]).toInt();
	m_uTimeSlice = m_pSettings->value(SETTINGS_KEY_TIMESLICE, 
		            m_qmParamDefaults[SETTINGS_KEY_TIMESLICE]).toInt();
	m_qsPgConnStr=m_pSettings->value(SETTINGS_KEY_PGCONN, 
		           m_qmParamDefaults[SETTINGS_KEY_PGCONN]).toString();
	QString qsFilePath=m_pSettings->value(SETTINGS_SQLITE_FILE,
		                m_qmParamDefaults[SETTINGS_SQLITE_FILE]).toString();
	QFile qfSqliteDb(qsFilePath);
	QFileInfo fiSqliteDb(qfSqliteDb);
    if (fiSqliteDb.isFile() && fiSqliteDb.isReadable()) {
        QDir qdSqliteDb=fiSqliteDb.absoluteDir();
        QDir qdCur=QDir::current();
        if (qdCur.rootPath()==qdSqliteDb.rootPath()) { // same Windows drive
            m_qsSqliteFilePath=qdCur.relativeFilePath(fiSqliteDb.absoluteFilePath());
        }
        else { // different Windows drives
            m_qsSqliteFilePath=fiSqliteDb.absoluteFilePath();
        }
    }

	m_qsMainctrlCfg=m_pSettings->value(SETTINGS_KEY_MAINCTRL,
		             m_qmParamDefaults[SETTINGS_KEY_MAINCTRL]).toString();
    QKalmanFilter::m_dFltSigmaS2=m_pSettings->value(SETTINGS_KEY_FLT_SIGMAS,
		                         m_qmParamDefaults[SETTINGS_KEY_FLT_SIGMAS]).toDouble(); // flt system speed sigma2 m2/s3
    QKalmanFilter::m_dFltSigmaM=m_pSettings->value(SETTINGS_KEY_FLT_SIGMAM,
		                         m_qmParamDefaults[SETTINGS_KEY_FLT_SIGMAM]).toDouble(); // flt measurement sigma m
    QKalmanFilter::m_dFltHeight=m_pSettings->value(SETTINGS_KEY_FLT_HEIGHT,
		                         m_qmParamDefaults[SETTINGS_KEY_FLT_HEIGHT]).toDouble(); // flt measurement sigma m
    QKalmanFilter::m_dStrobSpreadMin=m_pSettings->value(SETTINGS_KEY_FLT_MINSTROB,
		                              m_qmParamDefaults[SETTINGS_KEY_FLT_MINSTROB]).toDouble(); // flt strob size km
    QKalmanFilter::m_dStrobSpreadSpeed=m_pSettings->value(SETTINGS_KEY_FLT_STROBSPREAD,
		                                m_qmParamDefaults[SETTINGS_KEY_FLT_STROBSPREAD]).toDouble(); // flt strob spread m/s
    QKalmanFilter::m_iChi2Prob=m_pSettings->value(SETTINGS_KEY_FLT_CHI2PROB,
		                        m_qmParamDefaults[SETTINGS_KEY_FLT_CHI2PROB]).toInt(); // chi2 probability threshold
    QKalmanFilter::m_bUseMatRelax=m_pSettings->value(SETTINGS_KEY_FLT_USERELAX,
		                           m_qmParamDefaults[SETTINGS_KEY_FLT_USERELAX]).toBool(); // use cov relaxation
    QKalmanFilter::m_dMatRelaxTime=m_pSettings->value(SETTINGS_KEY_FLT_MATRELAX,
                                    m_qmParamDefaults[SETTINGS_KEY_FLT_MATRELAX]).toDouble(); // cov relaxation time
    QKalmanFilter::m_bStickTrajToModeS=m_pSettings->value(SETTINGS_KEY_FLT_STICKMODES,
                                        m_qmParamDefaults[SETTINGS_KEY_FLT_STICKMODES]).toBool(); // stick trajectory to ModeS
    QKalmanFilter::m_dClusterCutoff=m_pSettings->value(SETTINGS_KEY_FLT_RCLUSTER,
                                     m_qmParamDefaults[SETTINGS_KEY_FLT_RCLUSTER]).toDouble(); // cluster cutoff km
    QKalmanFilter::m_iClusterMinSize=m_pSettings->value(SETTINGS_KEY_FLT_CLUSTERSZ,
                                      m_qmParamDefaults[SETTINGS_KEY_FLT_CLUSTERSZ]).toInt(); // min cluster size to start traj
    QKalmanFilter::m_dTrajMaxVelocity=m_pSettings->value(SETTINGS_KEY_FLT_TRAJVMAX,
                                       m_qmParamDefaults[SETTINGS_KEY_FLT_TRAJVMAX]).toDouble(); // velocity (m/s) to kill traj
    QKalmanFilter::m_dTrajTimeout=m_pSettings->value(SETTINGS_KEY_FLT_TRAJTIMEOUT,
                                   m_qmParamDefaults[SETTINGS_KEY_FLT_TRAJTIMEOUT]).toDouble(); // timout (min) to kill traj
    QKalmanFilter::m_bEstIniVelocity=m_pSettings->value(SETTINGS_KEY_FLT_ESTINIVELO,
                                      m_qmParamDefaults[SETTINGS_KEY_FLT_ESTINIVELO]).toBool(); // estimate ini velocity from pri cluster
	QVoiProcessor::m_bUseGen=m_pSettings->value(SETTINGS_KEY_USE_GEN,
		                      m_qmParamDefaults[SETTINGS_KEY_USE_GEN]).toBool(); // use imitator instead of POITE db
	QVoiProcessor::m_dTrajDuration=m_pSettings->value(SETTINGS_KEY_TRAJDUR,
		                            m_qmParamDefaults[SETTINGS_KEY_TRAJDUR]).toDouble(); // traj duration min
	QPsVoi::m_Tolerance=m_pSettings->value(SETTINGS_KEY_FLT_TOLERANCE,
		                 m_qmParamDefaults[SETTINGS_KEY_FLT_TOLERANCE]).toDouble(); // tolerance thresh
	QPsVoi::m_UseTolerance=m_pSettings->value(SETTINGS_KEY_FLT_USE_TOL,
		                    m_qmParamDefaults[SETTINGS_KEY_FLT_USE_TOL]).toInt(); // use tolerance
	QGenerator::m_dGenSigmaM=m_pSettings->value(SETTINGS_KEY_GEN_SIGMAM,
		                         m_qmParamDefaults[SETTINGS_KEY_GEN_SIGMAM]).toDouble(); // gen measurement sigma m
	QGenerator::m_dGenDelay=m_pSettings->value(SETTINGS_KEY_GEN_DELAY,
		                         m_qmParamDefaults[SETTINGS_KEY_GEN_DELAY]).toDouble(); // gen measurement delay s
	QGenerator::m_dGenX0=m_pSettings->value(SETTINGS_KEY_GEN_X0,
		                     m_qmParamDefaults[SETTINGS_KEY_GEN_X0]).toDouble(); // gen init x0
	QGenerator::m_dGenY0=m_pSettings->value(SETTINGS_KEY_GEN_Y0,
	                         m_qmParamDefaults[SETTINGS_KEY_GEN_Y0]).toDouble(); // gen init y0
	QGenerator::m_dGenV0=m_pSettings->value(SETTINGS_KEY_GEN_V0,
		                     m_qmParamDefaults[SETTINGS_KEY_GEN_V0]).toDouble(); // gen init v
	QGenerator::m_dGenAz=m_pSettings->value(SETTINGS_KEY_GEN_AZ0,
		                     m_qmParamDefaults[SETTINGS_KEY_GEN_AZ0]).toDouble(); // gen init Az
	QGenerator::m_dGenTrajR=m_pSettings->value(SETTINGS_KEY_GEN_TRAJ_R,
		                        m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJ_R]).toDouble(); // gen traj R
	QGenerator::m_bGenTrajCW=m_pSettings->value(SETTINGS_KEY_GEN_TRAJ_CW,
		                         m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJ_CW]).toBool(); // gen traj CW
	QGenerator::m_dGenHeight=m_pSettings->value(SETTINGS_KEY_GEN_HEIGHT,
		                      m_qmParamDefaults[SETTINGS_KEY_GEN_HEIGHT]).toDouble(); // gen height km
	QGenerator::m_dGenKinkTime=m_pSettings->value(SETTINGS_KEY_GEN_KINKTIME,
		                      m_qmParamDefaults[SETTINGS_KEY_GEN_KINKTIME]).toDouble(); // gen kink time min
	QGenerator::m_dGenKinkAngle=m_pSettings->value(SETTINGS_KEY_GEN_KINKANGLE,
		                      m_qmParamDefaults[SETTINGS_KEY_GEN_KINKANGLE]).toDouble(); // gen kink angle deg
    QGenerator::m_iClusterMinSize=m_pSettings->value(SETTINGS_KEY_GEN_CLUSTERSZ,
                                   m_qmParamDefaults[SETTINGS_KEY_GEN_CLUSTERSZ]).toInt(); // min cluster size to start traj
    QGenerator::m_ttGenTrajectory=(QGenerator::TrajectoryTypes)
		                     m_pSettings->value(SETTINGS_KEY_GEN_TRAJTYPE,
		                      m_qmParamDefaults[SETTINGS_KEY_GEN_TRAJTYPE]).toInt(); // gen kink angle deg

	// Indicator geometry
	QString qsEncodedIndGeometry=m_pSettings->value(SETTINGS_KEY_IND_GEOMETRY, 
		                          m_qmParamDefaults[SETTINGS_KEY_IND_GEOMETRY]).toString();
	QRect qrIndGeometry = QSerial(qsEncodedIndGeometry).toRect(&bOk);
	if (!bOk) qrIndGeometry = QRect(200, 200, 200, 200);
	QIndicator::m_qrIndGeometry = qrIndGeometry;
	QIndicator::m_dScale=m_pSettings->value(SETTINGS_KEY_IND_SCALE,
		                  m_qmParamDefaults[SETTINGS_KEY_IND_SCALE]).toDouble(); // ind view scale
	QIndicator::m_dViewX0=m_pSettings->value(SETTINGS_KEY_IND_VIEWX0,
		                   m_qmParamDefaults[SETTINGS_KEY_IND_VIEWX0]).toDouble(); // ind view X0
	QIndicator::m_dViewY0=m_pSettings->value(SETTINGS_KEY_IND_VIEWY0,
		                   m_qmParamDefaults[SETTINGS_KEY_IND_VIEWY0]).toDouble(); // ind view Y0
	QIndicator::m_dGridStep=m_pSettings->value(SETTINGS_KEY_IND_GRIDSTEP,
		                     m_qmParamDefaults[SETTINGS_KEY_IND_GRIDSTEP]).toDouble(); // ind grid step

	m_dbeEngineSelected=(QPoiModel::POI_DB_ENGINES)
		m_pSettings->value(SETTINGS_KEY_DBENGINE,
	 	 m_qmParamDefaults[SETTINGS_KEY_DBENGINE]).toInt();
	//----- This must be done inside event loop to enable exceptions -----
	//if (m_dbeEngineSelected==QPoiModel::DB_Engine_Postgres) {
	//	m_pPoiModel->initPostgresDB(m_qsPgConnStr);
	//} else if (m_dbeEngineSelected==QPoiModel::DB_Engine_Sqlite) {
	//	if (qfSqliteDb.exists()) m_pPoiModel->initSqliteDB(qfSqliteDb);
	//}
	//--------------------------------------------------------------------
    QStringList qslNames=QIndicator::m_lsLegend.m_qlSettingNames;
	int iNumColors=QIndicator::m_lsLegend.m_iNumColors;
	QString qsSymbSizes=m_pSettings->value(SETTINGS_KEY_SYMBSIZES,
		                 m_qmParamDefaults[SETTINGS_KEY_SYMBSIZES]).toString();
    QByteArray baSymbSizes=QByteArray::fromBase64(qsSymbSizes.toLocal8Bit());
	QDataStream dsSymbSizes(&baSymbSizes,QIODevice::ReadWrite);
	for (int i=0; i<iNumColors; i++) {
		QString qsKey=qslNames.at(i);
		qsKey.replace(' ','_');
		QString qsEncodedColor=m_pSettings->value(qsKey,m_qmParamDefaults[qsKey]).toString();
        QIndicator::m_lsLegend.m_qlColors[i]=QSerial(qsEncodedColor).toColor(&bOk);
		if (!bOk) QIndicator::m_lsLegend.m_qlColors[i]=Qt::transparent;
		dsSymbSizes >> QIndicator::m_lsLegend.m_qlSizes[i];
	}
	m_pSettings->endGroup();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QTraceFilter::writeSettings() {
	m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
	QRect qrCurGeometry=geometry();

	// active prop page
	m_pSettings->setValue(SETTINGS_KEY_PROP_TAB,m_iActiveTab);

	m_pSettings->setValue(SETTINGS_KEY_GEOMETRY, QSerial(qrCurGeometry).toBase64());
	m_pSettings->setValue(SETTINGS_KEY_TICKIDX, m_uTickIdx);
	m_pSettings->setValue(SETTINGS_KEY_TIMESLICE, m_uTimeSlice);
	m_pSettings->setValue(SETTINGS_KEY_PGCONN, m_qsPgConnStr);
	m_pSettings->setValue(SETTINGS_SQLITE_FILE, m_qsSqliteFilePath);
	m_pSettings->setValue(SETTINGS_KEY_MAINCTRL, m_qsMainctrlCfg);

    m_pSettings->setValue(SETTINGS_KEY_FLT_SIGMAS,QKalmanFilter::m_dFltSigmaS2); // flt s system
    m_pSettings->setValue(SETTINGS_KEY_FLT_SIGMAM,QKalmanFilter::m_dFltSigmaM); // flt s meas
    m_pSettings->setValue(SETTINGS_KEY_FLT_HEIGHT,QKalmanFilter::m_dFltHeight); // flt tg height
	m_pSettings->setValue(SETTINGS_KEY_USE_GEN,QVoiProcessor::m_bUseGen); // use imitator
	m_pSettings->setValue(SETTINGS_KEY_TRAJDUR,QVoiProcessor::m_dTrajDuration); // traj duration min
	m_pSettings->setValue(SETTINGS_KEY_FLT_TOLERANCE,QPsVoi::m_Tolerance); // tolerance thresh
	m_pSettings->setValue(SETTINGS_KEY_FLT_USE_TOL,QPsVoi::m_UseTolerance); // use tolerance
    m_pSettings->setValue(SETTINGS_KEY_FLT_MINSTROB,QKalmanFilter::m_dStrobSpreadMin); // flt strob size km
    m_pSettings->setValue(SETTINGS_KEY_FLT_STROBSPREAD,QKalmanFilter::m_dStrobSpreadSpeed); // flt strob spread m/s
    m_pSettings->setValue(SETTINGS_KEY_FLT_CHI2PROB,QKalmanFilter::m_iChi2Prob); // chi2 probability threshold
    m_pSettings->setValue(SETTINGS_KEY_FLT_USERELAX,QKalmanFilter::m_bUseMatRelax); // use cov relaxation
    m_pSettings->setValue(SETTINGS_KEY_FLT_MATRELAX,QKalmanFilter::m_dMatRelaxTime); // cov relaxation time (sec)
    m_pSettings->setValue(SETTINGS_KEY_FLT_STICKMODES,QKalmanFilter::m_bStickTrajToModeS); // stick trajectory to ModeS
    m_pSettings->setValue(SETTINGS_KEY_FLT_RCLUSTER,QKalmanFilter::m_dClusterCutoff); // cluster cutoff radius km
    m_pSettings->setValue(SETTINGS_KEY_FLT_TRAJTIMEOUT,QKalmanFilter::m_dTrajTimeout); // traj timeout (min)
    m_pSettings->setValue(SETTINGS_KEY_FLT_TRAJVMAX,QKalmanFilter::m_dTrajMaxVelocity); // traj Vmax (m/s)
    m_pSettings->setValue(SETTINGS_KEY_FLT_CLUSTERSZ,QKalmanFilter::m_iClusterMinSize); // min cluster size to start traj
    m_pSettings->setValue(SETTINGS_KEY_FLT_ESTINIVELO,QKalmanFilter::m_bEstIniVelocity); // estimate ini velocity from pri cluster

	m_pSettings->setValue(SETTINGS_KEY_GEN_SIGMAM,QGenerator::m_dGenSigmaM); // gen s meas
	m_pSettings->setValue(SETTINGS_KEY_GEN_DELAY,QGenerator::m_dGenDelay); // gen meas delay
	m_pSettings->setValue(SETTINGS_KEY_GEN_X0,QGenerator::m_dGenX0); // gen init x0
	m_pSettings->setValue(SETTINGS_KEY_GEN_Y0,QGenerator::m_dGenY0); // gen init y0
	m_pSettings->setValue(SETTINGS_KEY_GEN_V0,QGenerator::m_dGenV0); // gen init v
	m_pSettings->setValue(SETTINGS_KEY_GEN_AZ0,QGenerator::m_dGenAz); // gen init Az
	m_pSettings->setValue(SETTINGS_KEY_GEN_TRAJ_R,QGenerator::m_dGenTrajR); // gen traj R
	m_pSettings->setValue(SETTINGS_KEY_GEN_TRAJ_CW,QGenerator::m_bGenTrajCW); // gen traj CW
	m_pSettings->setValue(SETTINGS_KEY_GEN_HEIGHT,QGenerator::m_dGenHeight); // gen target height
	m_pSettings->setValue(SETTINGS_KEY_GEN_KINKTIME,QGenerator::m_dGenKinkTime); // gen kink time
	m_pSettings->setValue(SETTINGS_KEY_GEN_KINKANGLE,QGenerator::m_dGenKinkAngle); // gen kink angle
    m_pSettings->setValue(SETTINGS_KEY_GEN_CLUSTERSZ,QGenerator::m_iClusterMinSize); // min cluster size to start traj
    m_pSettings->setValue(SETTINGS_KEY_GEN_TRAJTYPE,QGenerator::m_ttGenTrajectory); // gen traj type

	QRect qrIndGeometry = QIndicator::m_qrIndGeometry;
	m_pSettings->setValue(SETTINGS_KEY_IND_GEOMETRY, QSerial(qrIndGeometry).toBase64());
	m_pSettings->setValue(SETTINGS_KEY_IND_SCALE,QIndicator::m_dScale);
	m_pSettings->setValue(SETTINGS_KEY_IND_VIEWX0,QIndicator::m_dViewX0);
	m_pSettings->setValue(SETTINGS_KEY_IND_VIEWY0,QIndicator::m_dViewY0);
	m_pSettings->setValue(SETTINGS_KEY_IND_GRIDSTEP,QIndicator::m_dGridStep);

	m_pSettings->setValue(SETTINGS_KEY_DBENGINE,m_dbeEngineSelected);
    QStringList qslNames=QIndicator::m_lsLegend.m_qlSettingNames;
	QList<QColor> qlColors=QIndicator::m_lsLegend.m_qlColors;
	QList<int> qlSymbSizes=QIndicator::m_lsLegend.m_qlSizes;
	int iNumColors=QIndicator::m_lsLegend.m_iNumColors;
	QByteArray baSymbSizes;
	QDataStream dsSymbSizes(&baSymbSizes,QIODevice::ReadWrite);
	for (int i=0; i<iNumColors; i++) {
		QString qsKey=qslNames.at(i);
		qsKey.replace(' ','_');
	    m_pSettings->setValue(qsKey,QSerial(qlColors.at(i)).toBase64());
		dsSymbSizes << qlSymbSizes.at(i);
	}
	m_pSettings->setValue(SETTINGS_KEY_SYMBSIZES,QVariant(baSymbSizes.toBase64()));

	m_pSettings->endGroup();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UserControlInputFilter::UserControlInputFilter(QTraceFilter *pOwner, QObject *pobj /*=0*/) 
    : QObject(pobj), m_pOwner(pOwner) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*virtual*/ bool UserControlInputFilter::eventFilter(QObject *pobj, QEvent *pe) {
	QEvent::Type eventType = pe->type();
    // user input events
	if (eventType == QEvent::KeyPress) {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pe;
		// qDebug() << "pKeyEvent->key()=" << pKeyEvent->key() << " Qt::Key_Return=" << Qt::Key_Return ; 
		if (pKeyEvent->key()==Qt::Key_Return) {
			if (m_pOwner->m_bSetup) return false;
			QMetaObject::invokeMethod (
				m_pOwner,
				"onUpdate",
				Qt::QueuedConnection
			);
			// block all other events
			return true;
		}
		else if ((pKeyEvent->key()==Qt::Key_P) 
			   && (pKeyEvent->modifiers() & Qt::ControlModifier)) {
		    m_pOwner->onSetup();
			return true;
		}
		else if (pKeyEvent->key()==Qt::Key_Escape) {
		    m_pOwner->closeView();
			return true;
		}
		else if ((pKeyEvent->key()==Qt::Key_Plus || pKeyEvent->key()==Qt::Key_Minus
			   || pKeyEvent->key()==Qt::Key_Equal|| pKeyEvent->key()==Qt::Key_Bar ) 
			   && (pKeyEvent->modifiers() & Qt::ControlModifier)) {
		    //bool bZoomIn=false;
			//if (pKeyEvent->key()==Qt::Key_Plus || pKeyEvent->key()==Qt::Key_Equal) bZoomIn=true;
			//QMetaObject::invokeMethod (
			//	m_pOwner,
			//	"zoom",
			//	Qt::QueuedConnection,
			//	Q_ARG(bool, bZoomIn) );
			// block all other events
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
