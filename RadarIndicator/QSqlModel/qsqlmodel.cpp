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
void QSqlModel::closeDatabase() {
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(m_qsDBFile);
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
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isValid()) {
        qDebug() << "addDatabase failed";
        return false;
    }
    // QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection,false);
    db.setDatabaseName(m_qsDBFile);
    if (!db.open()) {
        qDebug() << "Cannot open database";
        return false;
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
bool QSqlModel::execQuery() {
    bool bOk;
    // double dGlobalMax2=0.0e0;
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return false;
    m_query = QSqlQuery(db);
    bOk=m_query.prepare("SELECT s.id AS guid, s.seqnum, s.ncnt, f.timestamp FROM"
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
bool QSqlModel::getStrobRecord(quint64 &iRecId, int &iStrob, int &iBeamCountsNum, qint64 &iTimestamp) {
    bool bOk;
    if (!m_query.isActive()) return false;
    if (!m_query.next()) return false;
    iRecId=m_query.value(m_record.indexOf("guid")).toInt(&bOk);
    if (!bOk) return false;
    iStrob=m_query.value(m_record.indexOf("seqnum")).toInt(&bOk);
    if (!bOk) return false;
    iBeamCountsNum=m_query.value(m_record.indexOf("ncnt")).toInt(&bOk);
    if (!bOk) return false;
    iTimestamp=m_query.value(m_record.indexOf("timestamp")).toLongLong(&bOk);
    if (!bOk) return false;
    return true;
}
//======================================================================================================
//
//======================================================================================================
bool QSqlModel::getBeamData(quint64 &iRecId, int &iBeam, QByteArray &baSamples) {
    bool bOk;
    QSqlDatabase db = QSqlDatabase::database();
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

