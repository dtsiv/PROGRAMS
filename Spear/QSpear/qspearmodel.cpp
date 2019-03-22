#include "qspearmodel.h"
#include "qspear.h"

#  define Q_DECL_IMPORT
#include "rmoexception.h"
#include "codogramsa.h"

#define MAX_LATLON 100.0e0
#define MAX_HEIGHT   1.0e4

double  QSpearModel::dMinFreq=9250.0, QSpearModel::dMaxFreq=9750.0, QSpearModel::dFreqStep=12.5;
double  QSpearModel::dMinAzimuth=-90.0, QSpearModel::dMaxAzimuth=90.0;
double  QSpearModel::dMinElevation=0.0, QSpearModel::dMaxElevation=90.0;
QString QSpearModel::placeholderFreq("9500.0"),
        QSpearModel::placeholderAzim("+90.0"),
        QSpearModel::placeholderElev("+90.0");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSpearModel::QSpearModel(QSpear *pOwner, QObject *parent/* =0 */)
		: QObject(parent)
	    , m_pSettings(NULL)
        , m_pOwner(pOwner)
        , m_dtWorkStart(QDateTime())
        , m_uWorkSecsTotal(0)
        , m_bConnected(false)
        , m_bInitError(false)
        , m_dASysLat(55.0)
        , m_dASysLon(37.0)
        , m_dASysHei(200.0)
        , m_dCanLat(55.0)
        , m_dCanLon(37.0)
        , m_dCanHei(200.0) { 
    this->m_cgCtrl.ctrlUnion1.ctrlStruct1.bIrr=false;
    this->m_cgCtrl.ctrlUnion1.ctrlStruct1.bPow=false;
    this->m_cgMode.modeUnion1.modeStruct1.bFnCtrl=false;
    this->m_cgMode.modeUnion1.modeStruct1.bWork=false;
    this->m_cgRotate.Direct=0;
    this->m_cgRotate.Speed=0;
    this->m_cgAntPos.usBeta=0;
    this->m_cgAntPos.usEpsilon=0;
	this->m_cgAntSensor.azimuth=0.0e0;
	this->m_cgAntSensor.azimuth=0.0e0;
    this->m_cgRotate.Direct=0;
    this->m_cgRotate.Speed=0;
	this->m_cgStatus.ud=0;
	this->m_cgStatus.pp4_1=0;
	this->m_cgStatus.pp4_2=0;    
	this->m_cgStatus.pp4_3=0;    
	this->m_cgStatus.pp4_4=0;       
	this->m_cgStatus.pp3=0;    
	this->m_cgStatus.cs1=0;      
	this->m_cgStatus.cs2=0;      
	this->m_cgStatus.gg=0;      
	this->m_cgStatus.su=0;       
	this->m_cgStatus.Tgr=0;       
	this->m_cgStatus.emi=0;      
	this->m_cgStatus.rdy=0;      
	this->m_cgStatus.enT=0;   

    this->m_staleStatus=this->m_cgStatus;
    m_bStaleStatus=true;
    this->m_qtStaleStatus.setInterval(TIMEOUT_STALE_STATUS);
    QObject::connect(&this->m_qtStaleStatus,SIGNAL(timeout()),SLOT(onStaleStatus()));
} 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSpearModel::~QSpearModel() { 
	QRect qrCurGeometry=m_pOwner->geometry();
    if (m_pSettings) {
	    m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
        m_pSettings->setValue(SETTINGS_KEY_GEOMETRY, QSerial(qrCurGeometry).toBase64());
        m_pSettings->setValue(SETTINGS_KEY_FREQUENCY, m_cgIni.freq);
        m_pSettings->setValue(SETTINGS_KEY_ANT_AZIM, m_cgIni.AzAnt);
        m_pSettings->setValue(SETTINGS_KEY_ANT_ELEV, m_cgIni.ElAnt);
        m_pSettings->setValue(SETTINGS_KEY_CANNON_AZIM, m_cgIni.AzCannon);
        m_pSettings->setValue(SETTINGS_KEY_CANNON_ELEV, m_cgIni.ElCannon);
        m_pSettings->setValue(SETTINGS_KEY_ROT_SPEED, m_cgRotate.Speed);
        m_pSettings->setValue(SETTINGS_KEY_IPCAM_URL, m_pOwner->m_qsIPCamURL);
	    m_pSettings->endGroup();
        delete m_pSettings;
    }
    writeCoordinates();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::isConnected() {
    return m_bConnected;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setConnected(bool bConnected) {
    m_bConnected=bConnected;
    emit changed(flConnected);
    if (m_bConnected) emit changed(flIni);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setIrradiation(bool bOn) {
    this->m_cgCtrl.ctrlUnion1.ctrlStruct1.bIrr=bOn;
    emit changed(flCTRL);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::startFK() {
    this->m_cgMode.modeUnion1.modeStruct1.bFnCtrl=true;
    this->m_cgMode.modeUnion1.modeStruct1.bWork=false;
    emit changed(flMODE);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::startStrelba() {
    this->m_cgMode.modeUnion1.modeStruct1.bFnCtrl=false;
    this->m_cgMode.modeUnion1.modeStruct1.bWork=true;
    emit changed(flMODE);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::svernuti() {
    // change azimuth to 0 in INI c.g.
    m_cgIni.AzAnt=m_cgAntPos.usBeta=0;
    // do not change elevation in INI c.g.
    m_cgAntPos.usEpsilon=m_cgIni.ElAnt;
    // save settings
    if (m_pSettings) {
	    m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
        m_pSettings->setValue(SETTINGS_KEY_ANT_AZIM, m_cgIni.AzAnt);
	    m_pSettings->endGroup();
    }
    emit changed(flAntPos);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString QSpearModel::qsFreq() {
    return QString::number(m_cgIni.freq,'f',1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setFreq(QString qsFreq) {
    bool bOk;
    double dFreq=qsFreq.toDouble(&bOk);
	double dChk=qBound(dMinFreq,dFreq,dMaxFreq)-dMinFreq;
	dChk=dMinFreq+dFreqStep*qRound(dChk/dFreqStep);
    if (!bOk || dFreq!=dChk) throw RmoException("Bad frequency input");
    if (dFreq==m_cgIni.freq) return;
    m_cgIni.freq=dFreq;
    if (m_pSettings) {
	    m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
        m_pSettings->setValue(SETTINGS_KEY_FREQUENCY, m_cgIni.freq);
	    m_pSettings->endGroup();
    }
    emit changed(flIni);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setRotDir(int iDir) { 
    if (iDir<0 || iDir>5) throw RmoException("Bad AS rotation direction");
    m_cgRotate.Direct=iDir;
    emit changed(flRotate);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setCannonDir() {
    if (m_pSettings) {
	    m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
        m_pSettings->setValue(SETTINGS_KEY_CANNON_AZIM, m_cgIni.AzCannon);
        m_pSettings->setValue(SETTINGS_KEY_CANNON_ELEV, m_cgIni.ElCannon);
	    m_pSettings->endGroup();
    }
    emit changed(flIni);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::setASysDir() {
    if (m_pSettings) {
	    m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
        m_pSettings->setValue(SETTINGS_KEY_ANT_AZIM, m_cgIni.AzAnt);
        m_pSettings->setValue(SETTINGS_KEY_ANT_ELEV, m_cgIni.ElAnt);
	    m_pSettings->endGroup();
    }
    emit changed(flIni);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString QSpearModel::formatFreqString(QString qsFreq) {
	bool bOk=false;
	double dFreq=qsFreq.toDouble(&bOk);
	if (!bOk) {
		return QString::number(dMinFreq,'f',1);
	}
	dFreq=qBound(dMinFreq,dFreq,dMaxFreq)-dMinFreq;
	dFreq=dMinFreq+dFreqStep*qRound(dFreq/dFreqStep);
	return QString::number(dFreq,'f',1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString QSpearModel::qsAzim() {
    return QString::number(this->m_cgIni.AzAnt,'f',1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString QSpearModel::qsElev() {
    return QString::number(this->m_cgIni.ElAnt,'f',1);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::onStaleStatus() {
    m_qtStaleStatus.stop();
    m_bStaleStatus=true;
    quint8 uEnT=m_cgStatus.enT;
    m_cgStatus = m_staleStatus;
    m_cgStatus.enT = uEnT;
    ChangeFlags fWhat=(ChangeFlags)(flStatus&(~flEnT));
    emit(changed(fWhat));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString QSpearModel::workTime(bool bTotal /*=false*/) {
    // total number of seconds
    int iSecs=0;
    int iSecsPerHour=3600;
    // total work time requested
    if (bTotal) iSecs=m_uWorkSecsTotal;
    // work is on now
    if (m_dtWorkStart.isValid()) {
        QDateTime dtCurrent = QDateTime::currentDateTime();
        iSecs += m_dtWorkStart.secsTo(dtCurrent);
    }
    // number of whole hours since new day
    int iHours=iSecs/iSecsPerHour;
    // seconds since new hour
    int iSecsSinceNewHour=iSecs%iSecsPerHour;
    // Format iSecsSinceNewDay as time string minues:seconds
    QTime qtDisplayTime(0,0);
    QString qsTime = qtDisplayTime.addSecs(iSecsSinceNewHour).toString("mm:ss");
    // append hours, if any
    return iHours?(QString("%1:%2").arg(iHours).arg(qsTime)):qsTime;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QByteArray * QSpearModel::getCodogramByType(int iType) {
    QByteArray *pba=0;
	ChangeFlags cfNotify=flNone;
    // input codograms
	if (iType == CGT_TEXT) { // 0x4000
        // not implemented
    }
	else if (iType == CGT_STATUS) { // 0x4001
        pba=new QByteArray(sizeof(CG_STATUS),0);
		CG_STATUS *pCg=(CG_STATUS *)pba->data();
        *pCg = m_cgStatus;
	}
	else if (iType == CGT_ANT_SENSOR) { // 0x4002
        pba=new QByteArray(sizeof(CG_ANT_SENSOR),0);
		CG_ANT_SENSOR *pCg=(CG_ANT_SENSOR *)pba->data();
        *pCg = m_cgAntSensor;
	}
    // output codograms
	else if (iType == CGT_CTRL) { // 0x2000
        pba=new QByteArray(sizeof(CG_CTRL),0);
		CG_CTRL *pCg=(CG_CTRL *)pba->data();
        *pCg = m_cgCtrl;
	}
	else if (iType == CGT_MODE) { // 0x2001
        pba=new QByteArray(sizeof(CG_MODE),0);
		CG_MODE *pCg=(CG_MODE *)pba->data();
        *pCg = m_cgMode;
	}
	else if (iType == CGT_ANT_POS) { // 0x2002
        pba=new QByteArray(sizeof(CG_ANT_POS),0);
		CG_ANT_POS *pCg=(CG_ANT_POS *)pba->data();
        *pCg = m_cgAntPos;
	}
	else if (iType == CGT_ROTATE) { // 0x2003
        pba=new QByteArray(sizeof(CG_ROTATE),0);
		CG_ROTATE *pCg=(CG_ROTATE *)pba->data();
        *pCg = m_cgRotate;
	}
	else if (iType == CGT_INI) { // 0x2004
        pba=new QByteArray(sizeof(CG_INI),0);
		CG_INI *pCg=(CG_INI *)pba->data();
        *pCg = m_cgIni;
	}
    if (!pba) throw RmoException(QString("Unknown codogram type %1").arg(iType,0,16));
    return pba;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::recvCodogram(int iType, QByteArray *pba) {
	if (iType == CGT_STATUS) {
		if (pba->size()!=sizeof(CG_STATUS)) {
			throw RmoException("ByteArray size mismatch for CGT_STATUS");
			return false;
		}
		CG_STATUS *pCg=(CG_STATUS *)pba->data();
        quint32 uWhat=(quint32)flNone;
        if (pCg->ud    != m_cgStatus.ud)    uWhat |= flUD01;
        if (pCg->pp4_1 != m_cgStatus.pp4_1) uWhat |= fl4PP01_1;
        if (pCg->pp4_2 != m_cgStatus.pp4_2) uWhat |= fl4PP01_2;
        if (pCg->pp4_3 != m_cgStatus.pp4_3) uWhat |= fl4PP01_3;
        if (pCg->pp4_4 != m_cgStatus.pp4_4) uWhat |= fl4PP01_4;
        if (pCg->pp3   != m_cgStatus.pp3)   uWhat |= fl3PP01;
        if (pCg->cs1   != m_cgStatus.cs1)   uWhat |= flCS01_1;
        if (pCg->cs2   != m_cgStatus.cs2)   uWhat |= flCS01_2;
        if (pCg->gg    != m_cgStatus.gg)    uWhat |= flGG01;
        if (pCg->su    != m_cgStatus.su)    uWhat |= flSU01;
        if (pCg->Tgr   != m_cgStatus.Tgr)   uWhat |= flTgr;
        // always receive emission status from server
        uWhat |= flEmi;
        // always receive ready status from server
        uWhat |= flRdy;
        if (pCg->enT   != m_cgStatus.enT)   uWhat |= flEnT;
		m_cgStatus = *pCg;
        if (uWhat) emit changed((ChangeFlags)uWhat);
        m_qtStaleStatus.stop();
        m_qtStaleStatus.start();
        m_bStaleStatus=false;
	}
    else if (iType == CGT_ANT_SENSOR) {
		if (pba->size()!=sizeof(CG_ANT_SENSOR)) {
			throw RmoException("ByteArray size mismatch for CGT_ANT_SENSOR");
			return false;
		}
		CG_ANT_SENSOR *pCg=(CG_ANT_SENSOR *)pba->data();
        // set local copy of c.g. ANT_SENSOR
		m_cgAntSensor = *pCg;
        // update local copy of c.g. INI
		m_cgIni.AzAnt=1.0e0*m_cgAntSensor.azimuth;
		m_cgIni.ElAnt=1.0e0*m_cgAntSensor.elevation;
        // save settings
        if (m_pSettings) {
	        m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
            m_pSettings->setValue(SETTINGS_KEY_ANT_AZIM, m_cgIni.AzAnt);
            m_pSettings->setValue(SETTINGS_KEY_ANT_ELEV, m_cgIni.ElAnt);
	        m_pSettings->endGroup();
        }
        emit changed(flAntSens);
    }
	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::readSettings() {
	QFile qfSettings(QDir::current().absolutePath()+"/qspear.xml");
	QSettings::Format XmlFormat = QSerial::registerXmlFormat();
    if (XmlFormat == QSettings::InvalidFormat) {
        m_qsErrorMessage = "QSerial::registerXmlFormat() failed";
        return false;
    } 
    else if (!qfSettings.exists()) {
        m_qsErrorMessage = QString("File %1 does not exist").arg(qfSettings.fileName());
        m_bInitError = true;
        return false;
    } 
    else if (!qfSettings.open(QIODevice::ReadWrite)) {
        m_qsErrorMessage = QString("File %1 cannot be opened").arg(qfSettings.fileName());
        m_bInitError = true;
        return false;
    }
    else {
		qfSettings.close();
		QFileInfo fiSettings(qfSettings);
		m_pSettings = new QSettings(fiSettings.absoluteFilePath(),XmlFormat);
	}
    // default values of parameters
	m_qmParamDefaults[SETTINGS_KEY_GEOMETRY]            = QVariant(QSerial(QRect(100,100,800,600)).toBase64());
	m_qmParamDefaults[SETTINGS_KEY_SRV_ADDRESS]         = QVariant(QString("127.0.0.1"));
	m_qmParamDefaults[SETTINGS_KEY_SRV_PORT]            = QVariant(QString("2001"));
	m_qmParamDefaults[SETTINGS_KEY_FREQUENCY]           = QVariant(9500.0);
	m_qmParamDefaults[SETTINGS_KEY_CANNON_AZIM]         = QVariant(12.1);
	m_qmParamDefaults[SETTINGS_KEY_CANNON_ELEV]         = QVariant(23.0);
	m_qmParamDefaults[SETTINGS_KEY_ANT_AZIM]            = QVariant(12.1);
	m_qmParamDefaults[SETTINGS_KEY_ANT_ELEV]            = QVariant(23.0);
	m_qmParamDefaults[SETTINGS_KEY_BLH_FILENAME]        = QVariant(QString("coordinate.txt"));
    m_qmParamDefaults[SETTINGS_KEY_ROT_SPEED]           = QVariant(1);

    // "http://user:passw123@192.168.100.16:80/Streaming/Channels/1/picture"
	// "http://cam.sokolniki.com:201/axis-cgi/jpg/image.cgi?fps=4&resolution=800x600"
	// "http://95.215.176.83:10090/image101.jpg?resolution=730x410&fps="
    // "http://62.117.66.226:5118/jpg/image.jpg"

    m_qmParamDefaults[SETTINGS_KEY_IPCAM_URL]           = QVariant(QString("http://62.117.66.226:5118/jpg/image.jpg"));

    // read seetings
	bool bOk;
	m_pSettings->beginGroup(SETTINGS_GROUP_NAME);
	// front panel geometry
	QString qsEncodedGeometry=m_pSettings->value(SETTINGS_KEY_GEOMETRY, 
		                       m_qmParamDefaults[SETTINGS_KEY_GEOMETRY]).toString();
	QRect qrGeometry = QSerial(qsEncodedGeometry).toRect(&bOk);
	if (!bOk) qrGeometry = QRect(200, 200, 200, 200);
    m_pOwner->setGeometry(qrGeometry);

	m_pOwner->m_qsServerAddress=m_pSettings->value(SETTINGS_KEY_SRV_ADDRESS,
		             m_qmParamDefaults[SETTINGS_KEY_SRV_ADDRESS]).toString();
	m_pOwner->m_iServerPort=m_pSettings->value(SETTINGS_KEY_SRV_PORT,
		             m_qmParamDefaults[SETTINGS_KEY_SRV_PORT]).toInt();
	m_pOwner->m_qsIPCamURL=m_pSettings->value(SETTINGS_KEY_IPCAM_URL,
		             m_qmParamDefaults[SETTINGS_KEY_IPCAM_URL]).toString();
    m_cgIni.freq      = m_pSettings->value(SETTINGS_KEY_FREQUENCY,
                         m_qmParamDefaults[SETTINGS_KEY_FREQUENCY]).toDouble();
    m_cgIni.AzAnt     = m_pSettings->value(SETTINGS_KEY_ANT_AZIM,
                         m_qmParamDefaults[SETTINGS_KEY_ANT_AZIM]).toDouble();
    m_cgIni.ElAnt     = m_pSettings->value(SETTINGS_KEY_ANT_ELEV,
                         m_qmParamDefaults[SETTINGS_KEY_ANT_ELEV]).toDouble();
    m_cgIni.AzCannon  = m_pSettings->value(SETTINGS_KEY_CANNON_AZIM,
                         m_qmParamDefaults[SETTINGS_KEY_CANNON_AZIM]).toDouble();
    m_cgIni.ElCannon  = m_pSettings->value(SETTINGS_KEY_CANNON_ELEV,
                         m_qmParamDefaults[SETTINGS_KEY_CANNON_ELEV]).toDouble();
    m_cgRotate.Speed = m_pSettings->value(SETTINGS_KEY_ROT_SPEED,
                        m_qmParamDefaults[SETTINGS_KEY_ROT_SPEED]).toInt();
	m_pSettings->endGroup();
 
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::readCoordinates() {
    // initialize geo coordinates 
	m_qfCoordinates.setFileName(QDir::current().absolutePath()+"/coordinates.txt");
    if (!m_qfCoordinates.exists()) {
        m_qsErrorMessage = QString("File %1 does not exist").arg(m_qfCoordinates.fileName());
        m_bInitError = true; return false;
    } 
    else if (!m_qfCoordinates.open(QIODevice::ReadWrite)) {
        m_qsErrorMessage = QString("File %1 cannot be opened").arg(m_qfCoordinates.fileName());
        m_bInitError = true; return false;
    }
    QByteArray baCoord = m_qfCoordinates.readAll();
    if (baCoord.isEmpty()) {
        m_qsErrorMessage = QString("File %1 is empty").arg(m_qfCoordinates.fileName());
        m_bInitError = true; return false;
    }
    QStringList qslCoord = QString(baCoord.data()).split(QRegExp("\\s+|\\n"),QString::SkipEmptyParts);
    if (!setCoordinates(qslCoord)) {
        m_qsErrorMessage = QString("File %1 format error").arg(m_qfCoordinates.fileName());
        m_bInitError = true; return false;
    }
    m_qfCoordinates.close();
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QStringList QSpearModel::getCoordinates() {
    QStringList retVal;
    retVal << QString::number(m_dASysLat,'f',6);
    retVal << QString::number(m_dASysLon,'f',6);
    retVal << QString::number(m_dASysHei,'f',0);
    retVal << QString::number(m_dCanLat,'f',6);
    retVal << QString::number(m_dCanLon,'f',6);
    retVal << QString::number(m_dCanHei,'f',0);
    return retVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::writeCoordinates() {
    if (!m_qfCoordinates.open(QIODevice::ReadWrite)) {
        m_qsErrorMessage = QString("File %1 cannot be opened").arg(m_qfCoordinates.fileName());
        return false;
    }
    m_qfCoordinates.resize(0);
    QString qsCoord("%1 %2 %3\n%4 %5 %6");
    int iRet=m_qfCoordinates.write(
        qsCoord.arg(m_dASysLat,9,'f',6,' ')
               .arg(m_dASysLon,9,'f',6,' ')
               .arg(m_dASysHei,4,'f',0,' ')
               .arg(m_dCanLat,9,'f',6,' ')
               .arg(m_dCanLon,9,'f',6,' ')
               .arg(m_dCanHei,4,'f',0,' ')
               .toLocal8Bit());
    m_qfCoordinates.close();
    if (iRet==-1) return false;
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSpearModel::setCoordinates(QStringList qslCoord) {
    if (qslCoord.size()<6) return false;
    // antenna system
    bool bOk=true, bOk1;
    m_dASysLat=qslCoord.at(0).toDouble(&bOk1); bOk&=bOk1;
    m_dASysLon=qslCoord.at(1).toDouble(&bOk1); bOk&=bOk1;
    m_dASysHei=qslCoord.at(2).toDouble(&bOk1); bOk&=bOk1;
    m_dCanLat=qslCoord.at(3).toDouble(&bOk1); bOk&=bOk1;
    m_dCanLon=qslCoord.at(4).toDouble(&bOk1); bOk&=bOk1;
    m_dCanHei=qslCoord.at(5).toDouble(&bOk1); bOk&=bOk1;
    bOk &= (m_dASysLat < MAX_LATLON);
    bOk &= (m_dASysLat >-MAX_LATLON);
    bOk &= (m_dASysLon < MAX_LATLON);
    bOk &= (m_dASysLon >-MAX_LATLON);
    bOk &= (m_dASysHei < MAX_HEIGHT);
    bOk &= (m_dASysHei >-MAX_HEIGHT);
    bOk &= (m_dCanLat < MAX_LATLON);
    bOk &= (m_dCanLat >-MAX_LATLON);
    bOk &= (m_dCanLon < MAX_LATLON);
    bOk &= (m_dCanLon >-MAX_LATLON);
    bOk &= (m_dCanHei < MAX_HEIGHT);
    bOk &= (m_dCanHei >-MAX_HEIGHT);
    if (!bOk) {
        m_qsErrorMessage = QString("Geo coordinates error: %1").arg(qslCoord.join(" "));
        return false;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpearModel::startWork(bool bStart /*=true*/) {
    QDateTime dtCurrent = QDateTime::currentDateTime();
    if (bStart) {
        m_dtWorkStart=dtCurrent;
    }
    else {
        m_uWorkSecsTotal+=m_dtWorkStart.secsTo(dtCurrent);
        m_dtWorkStart=QDateTime();
    }
}
