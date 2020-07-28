#include "qsqlmodel.h"
#include "qinisettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSqlModel::QSqlModel() : QObject(0)
        , m_qsDBFile("Uninitialized") {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QSQLMODEL_SQLITE_FILE,"ChronicaParser.sqlite3");
    m_qsDBFile = iniSettings.value(QSQLMODEL_SQLITE_FILE,scRes).toString();
    if (!QSqlDatabase::contains("QSQLITE")) {
        QSqlDatabase::addDatabase("QSQLITE");
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSqlModel::~QSqlModel() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(QSQLMODEL_SQLITE_FILE, m_qsDBFile);
    closeDatabase();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QHBoxLayout *pHLayout=new QHBoxLayout;
    pPropPages->m_pleDBFileName=new QLineEdit(m_qsDBFile);
    pHLayout->addWidget(new QLabel("DB file name"));
    pHLayout->addWidget(pPropPages->m_pleDBFileName);
    QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseSqliteFile()));
    QObject::connect(pPropPages,SIGNAL(chooseSqliteFile()),SLOT(onSQLiteFileChoose()));
    pHLayout->addWidget(ppbChoose);
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pHLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QSQLMODEL_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    m_qsDBFile = pPropPages->m_pleDBFileName->text();
    // qDebug() << "m_qsDBFile = " << m_qsDBFile;
}
//======================================================================================================
//
//======================================================================================================
void QSqlModel::startTransaction() {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) {
        qDebug() << "database is not open";
        return;
    }
    db.transaction();
    return;
}
//======================================================================================================
//
//======================================================================================================
void QSqlModel::commitTransaction() {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) {
        qDebug() << "database is not open";
        return;
    }
    db.commit();
    return;
}
//======================================================================================================
//
//======================================================================================================
void QSqlModel::closeDatabase() {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.open()) {
        qDebug() << "database is not open";
        return;
    }
    db.close();
    return;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::openDatabase() {
    bool bOk;
    QSqlDatabase db=QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isValid()) {
        qDebug() << "QSqlModel::openDatabase(): addDatabase()";
        db=QSqlDatabase::addDatabase("QSQLITE");
        if (!db.isValid()) {
            qDebug() << "addDatabase failed";
            return false;
        }
    }
    // if database is open - just check scheme
    if (!db.isOpen()) {
        db.setDatabaseName(m_qsDBFile);
        if (!db.open()) {
            qDebug() << "Cannot open database";
            return false;
        }
    }
    QSqlQuery query("SELECT COUNT(*) AS cnt FROM sqlite_master"
                    " WHERE type='table' AND name"
                    " IN ('files','strobs','samples','dbver')",db);
    QSqlRecord rec = query.record();
    int iFld=rec.indexOf("cnt");
    if (!query.next() || iFld==-1) return false;
    int iCnt=query.value(iFld).toInt(&bOk);
    if (!bOk) return false;

    // DB scheme version
    QString qsVersion;
    bOk= query.exec("SELECT version FROM dbver LIMIT 1");
    rec = query.record();
    int iVer=rec.indexOf("version");
    if (query.next() && iVer!=-1) {
        qsVersion=query.value(iVer).toString();
    }

    // If DB format is ill (wrong tables, version etc) then drop all and create anew
    if (iCnt!=4 || qsVersion!=DATA_BASE_VERSION) {
        return false;
    }
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::changeDatabaseName(QString qsDbFileName) {
    // check if file exists
    bool bNewFile=false;
    QFileInfo fiDbFile(qsDbFileName);
    if (!fiDbFile.exists() || !fiDbFile.isReadable()) {
        QFile qfDbFile(fiDbFile.absoluteFilePath());
        if (!qfDbFile.setPermissions(
                    QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                    QFileDevice::ReadUser | QFileDevice::WriteUser |
                    QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                    QFileDevice::ReadOther | QFileDevice::WriteOther)) {
            qDebug() << "setPermissions failed for " << fiDbFile.absoluteFilePath();
            return false;
        }
        if (!qfDbFile.open(QIODevice::WriteOnly)) {
            qDebug() << "DB file open failed";
            return false;
        }
        bNewFile=true;
        qfDbFile.close();
    }
    // set bNewFile flag for new and empty files
    if (fiDbFile.size()==0) bNewFile=true;
    // close current DB connection
    closeDatabase();
    QSqlDatabase db=QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isValid()) {
        qDebug() << "QSqlModel::changeDatabaseName(): addDatabase()";
        db=QSqlDatabase::addDatabase("QSQLITE");
        if (!db.isValid()) {
            qDebug() << "addDatabase failed";
            return false;
        }
    }
    // try to open new database
    db.setDatabaseName(qsDbFileName); // we just closed the QSqlDatabase::defaultConnection
    if (!db.open()) {
        qDebug() << "Cannot open database";
        return false;
    }
    if (bNewFile) {
        int iRet=createTables();
        if (iRet) {
            qDebug() << "createTables()= " << iRet << " failed for new file " << fiDbFile.absoluteFilePath();
            return false;
        }
    }
    db.close();
    // on success change QSqlModel class member m_qsDBFile
    m_qsDBFile=qsDbFileName;
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::execQuery() {
    bool bOk;
    // double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) return false;
    m_query = QSqlQuery(db);
    bOk=m_query.prepare("SELECT f.filever,s.id AS guid, s.seqnum,"
                    " s.ncnt, f.timestamp, s.structStrobData FROM"
                    " strobs s LEFT JOIN files f ON f.id=s.fileid"
                    " ORDER BY s.seqnum ASC"
                    );
    if (!bOk) return false;
    if (!m_query.exec()) return false;
    m_record = m_query.record();
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::getStrobRecord(quint64 &iRecId, int &iStrob, int &iBeamCountsNum, qint64 &iTimestamp,
                               QByteArray &baStructStrobeData, quint32 &uFileVer) {
    bool bOk;
    // QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    // if (!db.isOpen()) {
    //     qDebug() << "getStrobRecord(): DB is not open";
    // }
    if (!m_query.isActive()) {
    //    qDebug() << "getStrobRecord(): m_query is not active";
        return false;
    }
    if (!m_query.next()) {
    //    qDebug() << "getStrobRecord(): m_query.next() failed";
        return false;
    }
    iRecId=m_query.value(m_record.indexOf("guid")).toInt(&bOk);
    if (!bOk) return false;
    iStrob=m_query.value(m_record.indexOf("seqnum")).toInt(&bOk);
    if (!bOk) return false;
    iBeamCountsNum=m_query.value(m_record.indexOf("ncnt")).toInt(&bOk);
    if (!bOk) return false;
    iTimestamp=m_query.value(m_record.indexOf("timestamp")).toLongLong(&bOk);
    if (!bOk) return false;
    baStructStrobeData=m_query.value(m_record.indexOf("structStrobData")).toByteArray();
    if (baStructStrobeData.isEmpty()) return false;
    QString qsFileVer=m_query.value(m_record.indexOf("filever")).toString();
    uFileVer=qsFileVer.toUInt(&bOk);
    if (!bOk) return false;
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::getBeamData(quint64 &iRecId, int &iBeam, QByteArray &baSamples) {
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) return false;
    QSqlQuery queryData = QSqlQuery(db);
    bOk=queryData.prepare("SELECT sa.complexdata AS complexdata FROM"
                    " samples sa WHERE sa.strobid = :strobid AND sa.beam = :beam");
    if (!bOk) return false;
    queryData.bindValue(":strobid",iRecId);
    queryData.bindValue(":beam",iBeam);
    if (!queryData.exec()) return false;
    if (!queryData.next()) return false;
    QSqlRecord recData = queryData.record();
    baSamples = queryData.value(recData.indexOf("complexdata")).toByteArray();
    if (queryData.next()) return false;
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::getTotStrobes(quint32 &iTotStrobes) {
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) return false;
    QSqlQuery queryData = QSqlQuery(db);
    bOk=queryData.prepare("SELECT SUM(nstrobs) AS totStrobes FROM files");
    if (!bOk) return false;
    if (!queryData.exec()) return false;
    if (!queryData.next()) return false;
    QSqlRecord recData = queryData.record();
    int iRetVal = queryData.value(recData.indexOf("totStrobes")).toInt(&bOk);
    if (!bOk) return false;
    if (queryData.next()) return false;
    iTotStrobes=iRetVal;
    return true;
}
//======================================================================================================
//
//======================================================================================================
int QSqlModel::dropTables() {
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) {
        db.setDatabaseName(m_qsDBFile);
        if (!db.open()) {
            qDebug() << "Cannot open database";
            return 1;
        }
    }
    if (!db.isOpen()) return 2;
    QSqlQuery query(db);
    bOk= query.exec("DROP TABLE IF EXISTS samples;");
    if (!bOk) return 3;
    bOk= query.exec("DROP TABLE IF EXISTS strobs;");
    if (!bOk) return 4;
    bOk= query.exec("DROP TABLE IF EXISTS files;");
    if (!bOk) return 5;
    bOk= query.exec("DROP TABLE IF EXISTS dbver;");
    if (!bOk) return 6;
    qDebug() << "drop tables ok";
    return 0;

}
//======================================================================================================
//
//======================================================================================================
int QSqlModel::createTables() {
    bool bOk;

    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (!db.isOpen()) {
        db.setDatabaseName(m_qsDBFile);
        if (!db.open()) {
            qDebug() << "Cannot open database";
            return 1;
        }
    }
    if (!db.isOpen()) return 2;
    QSqlQuery query(db);
    bOk= query.exec("CREATE TABLE dbver (id INTEGER PRIMARY KEY ASC,"
                    " version TEXT);");
    if (!bOk) return 3;
    bOk = query.prepare("INSERT INTO dbver (version) VALUES (:version);");
    if (!bOk) return 4;
    query.bindValue(":version",DATA_BASE_VERSION);
    if (!query.exec()) return 5;

    bOk= query.exec("CREATE TABLE files (id INTEGER PRIMARY KEY ASC,"
                    " timestamp BIGINT,"
                    " filepath TEXT,"
                    " nstrobs INTEGER,"
                    " filever TEXT);");
    if (!bOk) return 6;
    bOk= query.exec("CREATE TABLE strobs (id INTEGER PRIMARY KEY ASC,"
                    " seqnum INTEGER,"
                    " ncnt INTEGER,"
                    " fileid INTEGER,"
                    " structStrobData BLOB,"
                    " FOREIGN KEY(fileid) REFERENCES files(id));");
    if (!bOk) return 7;
    bOk= query.exec("CREATE TABLE samples (id INTEGER PRIMARY KEY ASC,"
                    " strobid INTEGER,"
                    " beam INTEGER,"
                    " complexdata BLOB,"
                    " FOREIGN KEY(strobid) REFERENCES strobs(id));");
    if (!bOk) return 8;
    return 0;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::isDBOpen() {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    if (db.isOpen()) return true;
    return false;
}
//======================================================================================================
//
//======================================================================================================
qint64 QSqlModel::addFileRec(quint64 uTimeStamp, QString qsFilePath, int nStrobs, QString qsFileVer) {
    quint64 iFilesId;
    bool bOk;
    QSqlRecord rec;
    int iFld;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);

    QSqlQuery query(db);

    bOk = query.prepare("SELECT id AS filesid FROM files"
                       " WHERE timestamp=:timestamp AND filepath=:filepath;");
    if (!bOk) return -1;
    query.bindValue(":timestamp",uTimeStamp);
    query.bindValue(":filepath",qsFilePath);
    if (!query.exec()) return -1;
    rec = query.record();
    iFld=rec.indexOf("filesid");
    if (iFld==-1) {
        qDebug() << "failed: rec.indexOf(filesid)";
        return -1;
    }
    while (query.next()) {
        iFilesId=query.value(iFld).toLongLong(&bOk);
        qDebug() << "deleting iFilesId " << iFilesId;
        if (!bOk) return -1;
        QSqlQuery qDel(db);
        qDel.prepare("DELETE FROM samples"
                     " WHERE strobid IN (SELECT id FROM strobs s"
                     " WHERE s.fileid=:fileid);");
        qDel.bindValue(":fileid",iFilesId);
        if (!qDel.exec()) {
            qDebug() << "DELETE FROM samples failed";
            return -1;
        }
        qDel.prepare("DELETE FROM strobs"
                     " WHERE fileid=:fileid;");
        qDel.bindValue(":fileid",iFilesId);
        if (!qDel.exec()) return -1;
        qDel.prepare("DELETE FROM files"
                     " WHERE id=:fileid;");
        qDel.bindValue(":fileid",iFilesId);
        if (!qDel.exec()) {
            qDebug() << "DELETE FROM strobs failed";
            return -1;
        }
    }
    query.prepare("INSERT INTO files (timestamp,filepath,nstrobs,filever) VALUES"
                 " (:timestamp,:filepath,:nstrobs,:filever);");
    query.bindValue(":timestamp",uTimeStamp);
    query.bindValue(":filepath",qsFilePath);
    query.bindValue(":nstrobs",nStrobs);
    query.bindValue(":filever",qsFileVer);
    if (!query.exec()) {
        qDebug() << "INSERT INTO files failed";
        return -1;
    }
    query.prepare("SELECT id AS filesid FROM files"
                 " WHERE timestamp=:timestamp AND filepath=:filepath;");
    query.bindValue(":timestamp",uTimeStamp);
    query.bindValue(":filepath",qsFilePath);
    if (!query.exec()) {
        qDebug() << "SELECT filesid failed";
        return -1;
    }
    rec = query.record();
    iFld=rec.indexOf("filesid");
    if (iFld==-1) return -1;
    if (!query.next()) {
        qDebug() << "SELECT filesid returned empty set";
        return -1;
    }
    iFilesId=query.value(iFld).toLongLong(&bOk);
    if (!bOk) return -1;
    if (query.next()) {
        qDebug() << "SELECT filesid returned multiple entries";
        return -1;
    }
    return iFilesId;
}
//======================================================================================================
//
//======================================================================================================
qint64 QSqlModel::addStrobe(int strobeNo, int beamCountsNum, QByteArray baStructStrobeData, qint64 iFileId) {
    bool bOk;
    QSqlRecord rec;
    int iFld;
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);

    QSqlQuery query(db);
    query.prepare("INSERT INTO strobs (seqnum,ncnt,fileid,structStrobData) VALUES"
                 " (:seqnum,:ncnt,:fileid,:structStrobData);");
    query.bindValue(":seqnum",strobeNo);
    query.bindValue(":ncnt",beamCountsNum);
    //qDebug() << "iFileId=" << iFileId;
    query.bindValue(":fileid",iFileId);
    query.bindValue(":structStrobData",baStructStrobeData);
    //qDebug() << "query.exec(): INSERT";
    if (!query.exec()) return -1;
    query.prepare("SELECT id AS strobid FROM strobs"
                 " WHERE fileid=:fileid AND seqnum=:seqnum;");
    query.bindValue(":fileid",iFileId);
    query.bindValue(":seqnum",strobeNo);
    //qDebug() << "query.exec(): SELECT";
    if (!query.exec()) return -1;
    rec = query.record();
    iFld=rec.indexOf("strobid");
    //qDebug() << "iFld=" << iFld;
    if (iFld==-1) return -1;
    //qDebug() << "!query.next()";
    if (!query.next()) return -1;
    quint64 iStrobId=query.value(iFld).toLongLong(&bOk);
    //qDebug() << "query.value = " << iStrobId << " bOk=" << bOk;
    if (!bOk) return -1;
    // the query must return only one record. Otherwise return (-1)
    //qDebug() << "query.next()";
    if (query.next()) return -1;
    //qDebug() << "addStrobe returns: " << iStrobId;
    return iStrobId;
}
//======================================================================================================
//
//======================================================================================================
int QSqlModel::addSamples(qint64 iStrobId, int iBeam, char *pSamples, int iSize) {
    QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    // QSqlRecord rec;

    QSqlQuery query(db);
    query.prepare("INSERT INTO samples (strobid,beam,complexdata) VALUES"
                 " (:strobid,:beam,:complexdata);");
    query.bindValue(":strobid",iStrobId);
    query.bindValue(":beam",iBeam);
    query.bindValue(":complexdata",QByteArray(pSamples,iSize));
    if (!query.exec()) return 1;
    return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::onSQLiteFileChoose() {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());

    QFileDialog dialog(pPropPages);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("SQLite db file (*.db *.sqlite *.sqlite3)"));
    // if current DB file exists - set its directory in dialog
    QString qsCurrFile = pPropPages->m_pleDBFileName->text();
    QFileInfo fiCurrFile(qsCurrFile);
    if (fiCurrFile.absoluteDir().exists()) {
        dialog.setDirectory(fiCurrFile.absoluteDir());
    }
    else { // otherwise set current directory in dialog
        dialog.setDirectory(QDir::current());
    }
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable())	{
            QDir qdCur = QDir::current();
            QDir qdSelFilePath = fiSelFilePath.absoluteDir();
            if (qdCur.rootPath() == qdSelFilePath.rootPath()) { // same Windows drives
                pPropPages->m_pleDBFileName->setText(qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath()));
            }
            else { // different Windows drives
                pPropPages->m_pleDBFileName->setText(fiSelFilePath.absoluteFilePath());
            }
        }
        else {
            pPropPages->m_pleDBFileName->setText("error");
        }
    }
}
