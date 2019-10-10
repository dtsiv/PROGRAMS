#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "sqlmodel.h"
#include "parser.h"
#include "poi.h"

bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map);
bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map);

QMap<QString,QVariant>       qmParamDefaults;
QString                      qsDataFile;
QString                      qsOperation;
QDateTime                    dtTimeStamp;
QString                      qsSqliteDbName;

enum OperationModes omSelectedMode = mUndefinedMode;

//======================================================================================================
//
//======================================================================================================
int readSettings() {
    QFile qfSettings(QDir::current().absolutePath()+"/chronicaparser.xml");
    QSettings::Format XmlFormat = QSettings::registerFormat("xml", &readXmlFile, &writeXmlFile, Qt::CaseSensitive);
    if (XmlFormat == QSettings::InvalidFormat) return 1;
    else if (!qfSettings.exists()) return 2;
    else if (!qfSettings.open(QIODevice::ReadWrite)) return 3;
    else {
        qfSettings.close();
    }
    QFileInfo fiSettings(qfSettings);
    QSettings qSettings(fiSettings.absoluteFilePath(),XmlFormat);
    // default values of parameters
    qmParamDefaults[SETTINGS_KEY_DATAFILE]  = QString("C:/PROGRAMS/DATA/RegFixed/2019-03-01T14-12-02/00000000");
    qmParamDefaults[SETTINGS_KEY_OPERATION] = QString("Undefined");
    qmParamDefaults[SETTINGS_KEY_TSAMPL]    = 0.120e0; // usec
    qmParamDefaults[SETTINGS_KEY_FCARRIER]  = 1.0e4;   // MHz
    qmParamDefaults[SETTINGS_KEY_NTAU]      = 8;       // samples per pulse
    qmParamDefaults[SETTINGS_KEY_NT]        = 80;      // "samplesPerPeriod"
    qmParamDefaults[SETTINGS_KEY_NT_]       = 48;      // "recordedSamplesPerPeriod"
    qmParamDefaults[SETTINGS_KEY_NP]        = 1100;    // "periodsPerBatch"
    qmParamDefaults[SETTINGS_KEY_DBNAME]    = QString("/home/tsivlin/PROGRAMS/DATA/chronicaparser.sqlite3");

    // read settings
    bool bOk;
    qSettings.beginGroup(SETTINGS_GROUP_NAME);
    qsDataFile=qSettings.value(SETTINGS_KEY_DATAFILE,
               qmParamDefaults[SETTINGS_KEY_DATAFILE]).toString();
    qsOperation=qSettings.value(SETTINGS_KEY_OPERATION,
                qmParamDefaults[SETTINGS_KEY_OPERATION]).toString();
    iDopplerFrom=qSettings.value(SETTINGS_KEY_DOPPLERFROM,0).toInt(&bOk);
    iDopplerTo=qSettings.value(SETTINGS_KEY_DOPPLERTO,0).toInt(&bOk);
    iDelayFrom=qSettings.value(SETTINGS_KEY_DELAYFROM,0).toInt(&bOk);
    iDelayTo=qSettings.value(SETTINGS_KEY_DELAYTO,0).toInt(&bOk);
    dThreshold=qSettings.value(SETTINGS_KEY_THRESHOLD,0).toDouble(&bOk);
    dCarrierF=qSettings.value(SETTINGS_KEY_FCARRIER,
              qmParamDefaults[SETTINGS_KEY_FCARRIER]).toDouble(&bOk);
    dTs=qSettings.value(SETTINGS_KEY_TSAMPL,
        qmParamDefaults[SETTINGS_KEY_TSAMPL]).toDouble(&bOk);
    Ntau=qSettings.value(SETTINGS_KEY_NTAU,
         qmParamDefaults[SETTINGS_KEY_NTAU]).toInt(&bOk);
    NT=qSettings.value(SETTINGS_KEY_NT,
       qmParamDefaults[SETTINGS_KEY_NT]).toInt(&bOk);
    NT_=qSettings.value(SETTINGS_KEY_NT_,
        qmParamDefaults[SETTINGS_KEY_NT_]).toInt(&bOk);
    Np=qSettings.value(SETTINGS_KEY_NP,
       qmParamDefaults[SETTINGS_KEY_NP]).toInt(&bOk);
    dRelThresh=qSettings.value(SETTINGS_KEY_RELTHRESH,1.0).toDouble(&bOk);
    nFalseAlarms=qSettings.value(SETTINGS_KEY_FALSEALARMS,1).toDouble(&bOk);
    qsSqliteDbName=qSettings.value(SETTINGS_KEY_DBNAME,
                   qmParamDefaults[SETTINGS_KEY_DBNAME]).toString();
    iPlotSlicePeriod=qSettings.value(SETTINGS_KEY_SLICEPER,0).toInt(&bOk);
    iPlotSliceBeam=qSettings.value(SETTINGS_KEY_SLICEBEAM,0).toInt(&bOk);
    iRawStrobFrom=qSettings.value(SETTINGS_KEY_RAWFROM,0).toInt(&bOk);
    iRawStrobTo=qSettings.value(SETTINGS_KEY_RAWTO,0).toInt(&bOk);
    dMaxVelocity=qSettings.value(SETTINGS_KEY_MAXVELO,0).toDouble(&bOk);
    dFalseAlarmProb=qSettings.value(SETTINGS_KEY_PFALARM,1.0e-5).toDouble(&bOk);

    if (qsOperation=="dataImport") omSelectedMode=mDataImport;
    else if (qsOperation=="poi") omSelectedMode=mPrimaryProc;
    else if (qsOperation=="poi20190409") omSelectedMode=mPOI20190409;
    else if (qsOperation=="justDoppler") omSelectedMode=mJustDoppler;
    else if (qsOperation=="poiRaw") omSelectedMode=mPrimaryProcRaw;
    else if (qsOperation=="poiNoncoher") omSelectedMode=mPrimaryProcNoncoher;
    else if (qsOperation=="plotSignal") omSelectedMode=mSignalPlot;
    else if (qsOperation=="plotSignal3D") omSelectedMode=mSignalPlot3D;
    else omSelectedMode=mUndefinedMode;
    qSettings.endGroup();

    // save settings
    qSettings.beginGroup(SETTINGS_GROUP_NAME);
    qSettings.setValue(SETTINGS_KEY_DATAFILE,qsDataFile);
    qSettings.endGroup();
    qSettings.sync();
    return 0;
}

//======================================================================================================
//
//======================================================================================================
int openDataFile() {
    int iRetval;
    if ((iRetval=openDatabase())) return 200+iRetval;

    QTextStream                  tsStdOut(stdout);
    QFileInfo fiDataFile(qsDataFile);
    if (!fiDataFile.exists() || !fiDataFile.isFile()) return 1;
    QDir qdRoot=fiDataFile.absoluteDir();
    if (!qdRoot.cdUp()) return 2;
    tsStdOut << "browsing: " << qdRoot.absolutePath() << endl;

    QStringList qslEntries=qdRoot.entryList(QDir::Dirs,QDir::NoSort);
    if (!qslEntries.count()) return 3;
    for (int i=0; i<qslEntries.count(); i++) {
        QString qsCurDir = qslEntries.at(i);
        if (qsCurDir == "." || qsCurDir == ".." || qsCurDir.contains("2019-02-28")) continue;
        QDir qdCurDir = QDir(qdRoot.absoluteFilePath(qsCurDir));
        tsStdOut << "browsing: " << qdRoot.absoluteFilePath(qsCurDir) << endl;

        if (!qdCurDir.exists(DATA_FILE_NAME)) return 4;
        // open
        QString qsCurFile=qdCurDir.absoluteFilePath(DATA_FILE_NAME);
        QFile qfCurFile(qsCurFile);
        if (!qfCurFile.exists() || !qfCurFile.open(QIODevice::ReadOnly)) {
            tsStdOut << "\nCannot open file: " << QFileInfo(qfCurFile).absoluteFilePath()  << "\n\n";
            return 5;
        }

        // parse file name
        QStringList qslDirs=qsCurFile.split("/");
        if (qslDirs.count()<2) continue;
        QString qsTimeStamp=qslDirs.at(qslDirs.count()-2);
        QStringList qslDateTime=qsTimeStamp.split("T");
        if (qslDateTime.count()!=2) continue;
        dtTimeStamp=QDateTime(
            QDate::fromString(qslDateTime.at(0),"yyyy-MM-dd"),
            QTime::fromString(qslDateTime.at(1),"hh-mm-ss"),
            Qt::LocalTime);
        if (!dtTimeStamp.isValid()) continue;

        // map & parse contents
        char *pFileData=(char *)qfCurFile.map(0,qfCurFile.size());
        if (!pFileData) continue;
        qfCurFile.close();
        if ((iRetval=parseDataFile(dtTimeStamp.toMSecsSinceEpoch(),qsCurFile, pFileData, qfCurFile.size()))) return 100+iRetval;
        tsStdOut << "parsed: " << qsTimeStamp << endl;
    }
    return 0;
}
//======================================================================================================
//
//======================================================================================================
int openDatabase() {
    bool bOk;
    QTextStream                  tsStdOut(stdout);
    // QStringList qsl=QSqlDatabase::drivers();
    // for (int i=0; i<qsl.count(); i++) tsStdOut << qsl.at(i) << endl;

    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    db.setDatabaseName(qsSqliteDbName);
    if (!db.open()) {
        tsStdOut << "Cannot open database" << endl;
        return 1;
    }
    QSqlQuery query("SELECT COUNT(*) AS cnt FROM sqlite_master"
                    " WHERE type='table' AND name"
                    " IN ('files','strobs','samples')",db);
    QSqlRecord rec = query.record();
    int iFld=rec.indexOf("cnt");
    if (!query.next() || iFld==-1) return 3;
    int iCnt=query.value(iFld).toInt(&bOk);
    if (!bOk) return 4;
    if (iCnt!=3) return createTables();
    return 0;
}
//======================================================================================================
//
//======================================================================================================
int closeDatabase() {
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) db.close();
    return 0;
}
//======================================================================================================
//
//======================================================================================================
int createTables() {
    bool bOk;
    QTextStream                  tsStdOut(stdout);
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.isOpen()) return 1;
    QSqlQuery query(db);
    bOk= query.exec("CREATE TABLE files (id INTEGER PRIMARY KEY ASC,"
                    " timestamp BIGINT,"
                    " filepath TEXT,"
                    " nstrobs INTEGER);");
    if (!bOk) return 11;
    bOk= query.exec("CREATE TABLE strobs (id INTEGER PRIMARY KEY ASC,"
                    " seqnum INTEGER,"
                    " ncnt INTEGER,"
                    " fileid INTEGER,"
                    " FOREIGN KEY(fileid) REFERENCES files(id));");
    if (!bOk) return 12;
    bOk= query.exec("CREATE TABLE samples (id INTEGER PRIMARY KEY ASC,"
                    " strobid INTEGER,"
                    " beam INTEGER,"
                    " complexdata BLOB,"
                    " FOREIGN KEY(strobid) REFERENCES strobs(id));");
    if (!bOk) return 13;
    // tsStdOut << "create tables ok" << endl;
    return 0;
}
//======================================================================================================
//
//======================================================================================================
qint64 addFileRec(quint64 uTimeStamp, QString qsFilePath, int nStrobs) {
    quint64 iFilesId;
    bool bOk;
    QSqlRecord rec;
    int iFld;
    QTextStream                  tsStdOut(stdout);
    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    //tsStdOut << "QSqlDatabase().isOpen()=" << QSqlDatabase().isOpen() << endl;
    //tsStdOut << "db.isOpen()=" << db.isOpen() << endl;

    bOk = query.prepare("SELECT id AS filesid FROM files"
                       " WHERE timestamp=:timestamp;");
    if (!bOk) return -1;
    query.bindValue(":timestamp",uTimeStamp);
	if (!query.exec()) return -1;
    rec = query.record();
    iFld=rec.indexOf("filesid");
	if (iFld==-1) {
		tsStdOut << "failed: rec.indexOf(filesid)" << endl;
		return -1;
	}
    while (query.next()) {
        iFilesId=query.value(iFld).toLongLong(&bOk);
        if (!bOk) return -1;
        QSqlQuery qDel(db);
        qDel.prepare("DELETE FROM samples"
                     " WHERE strobid IN (SELECT id FROM strobs s"
                     " WHERE s.fileid=:fileid);");
        qDel.bindValue(":fileid",iFilesId);
        // tsStdOut << "DELETE FROM samples" << endl;
        if (!qDel.exec()) return -1;
        qDel.prepare("DELETE FROM strobs"
                     " WHERE fileid=:fileid;");
        qDel.bindValue(":fileid",iFilesId);
        // tsStdOut << "DELETE FROM strobs" << endl;
        if (!qDel.exec()) return -1;
        qDel.prepare("DELETE FROM files"
                     " WHERE id=:fileid;");
        qDel.bindValue(":fileid",iFilesId);
        if (!qDel.exec()) return -1;
    }
    query.prepare("INSERT INTO files (timestamp,filepath,nstrobs) VALUES"
                 " (:timestamp,:filepath,:nstrobs);");
    query.bindValue(":timestamp",uTimeStamp);
    query.bindValue(":filepath",qsFilePath);
    query.bindValue(":nstrobs",nStrobs);
    if (!query.exec()) return -1;
    query.prepare("SELECT id AS filesid FROM files"
                 " WHERE timestamp=:timestamp;");
    query.bindValue(":timestamp",uTimeStamp);
    if (!query.exec()) return -1;
    rec = query.record();
    iFld=rec.indexOf("filesid");
    if (iFld==-1) return -1;
    if (!query.next()) return -1;
    iFilesId=query.value(iFld).toLongLong(&bOk);
    if (!bOk) return -1;
    if (query.next()) return -1;
    return iFilesId;
}
//======================================================================================================
//
//======================================================================================================
qint64 addStrobe(int strobeNo, int beamCountsNum, qint64 iFileId) {
    bool bOk;
    QSqlRecord rec;
    int iFld;
    QTextStream                  tsStdOut(stdout);
    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery query(db);
    // tsStdOut << "addStrobe: kp01" << endl;
    query.prepare("INSERT INTO strobs (seqnum,ncnt,fileid) VALUES"
                 " (:seqnum,:ncnt,:fileid);");
    query.bindValue(":seqnum",strobeNo);
    query.bindValue(":ncnt",beamCountsNum);
    query.bindValue(":fileid",iFileId);
    if (!query.exec()) return -1;
	// tsStdOut << "addStrobe: kp02" << endl;
    query.prepare("SELECT id AS strobid FROM strobs"
                 " WHERE fileid=:fileid AND seqnum=:seqnum;");
    query.bindValue(":fileid",iFileId);
    query.bindValue(":seqnum",strobeNo);
    if (!query.exec()) return -1;
	// tsStdOut << "addStrobe: kp03" << endl;
    rec = query.record();
    iFld=rec.indexOf("strobid");
    if (iFld==-1) return -1;
	// tsStdOut << "addStrobe: kp04" << endl;
    if (!query.next()) return -1;
    // tsStdOut << "addStrobe: kp05" << endl;
    quint64 iStrobId=query.value(iFld).toLongLong(&bOk);
    if (!bOk) return -1;
	//tsStdOut << "addStrobe: kp06" << endl;
    if (query.next()) return -1;
	//tsStdOut << "addStrobe: kp07" << endl;
    return iStrobId;
}
//======================================================================================================
//
//======================================================================================================
int addSamples(qint64 iStrobId, int iBeam, char *pSamples, int iSize) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlRecord rec;

    QSqlQuery query(db);
    query.prepare("INSERT INTO samples (strobid,beam,complexdata) VALUES"
                 " (:strobid,:beam,:complexdata);");
    query.bindValue(":strobid",iStrobId);
    query.bindValue(":beam",iBeam);
    query.bindValue(":complexdata",QByteArray(pSamples,iSize));
    if (!query.exec()) return 1;
    return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool readXmlFile(QIODevice &device, QSettings::SettingsMap &map) {
    QXmlStreamReader xmlReader(&device);
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if (xmlReader.isStartElement()) {
            if (xmlReader.name().toString()==SETTINGS_GROUP_NAME) {
                QString qsKey;
                while (xmlReader.readNext()!=QXmlStreamReader::Invalid) {
                    if (xmlReader.isStartElement()) {
                        qsKey=xmlReader.name().toString();
                    }
                    if (xmlReader.isEndDocument()) {
                        break;
                    }
                    if (xmlReader.isEndElement()) {
                        continue;
                    }
                    if (xmlReader.isCharacters() && !xmlReader.isWhitespace()) {
                        QString qsValue=xmlReader.text().toString();
                        map.insert(SETTINGS_GROUP_NAME+QString::fromLatin1("/")+qsKey,qsValue);
                        qsKey.clear();
                    }
                }
            }
        }
    }
    QMapIterator<QString, QVariant> i(map);
    if (xmlReader.hasError() && xmlReader.error()!=QXmlStreamReader::PrematureEndOfDocumentError) {
        QString qsErrMsg("(line %1, col %2, chr %3) :");
        qDebug() << "QSerial: xml parsing error @"
            << qsErrMsg
                 .arg(xmlReader.lineNumber())
                 .arg(xmlReader.columnNumber())
                 .arg(xmlReader.characterOffset())
            << xmlReader.errorString();
        return false;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool writeXmlFile(QIODevice &device, const QSettings::SettingsMap &map) {
    QXmlStreamWriter xmlWriter( &device );
    xmlWriter.setAutoFormatting( true );

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement(SETTINGS_GROUP_NAME);
    QSettings::SettingsMap::const_iterator mi = map.begin();
    for( mi; mi != map.end(); ++mi) {
        QString qsKey = mi.key();
        QStringList qslGroups = qsKey.split("/",QString::SkipEmptyParts,Qt::CaseSensitive);
        xmlWriter.writeStartElement(qslGroups.at(qslGroups.count()-1));
        xmlWriter.writeCharacters( mi.value().toString() );
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndDocument();

    return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QString qsGetFileName() {
    return qsDataFile;
}
