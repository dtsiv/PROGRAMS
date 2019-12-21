#include "qsqlmodel.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "qavtctrl.h"
#include "codograms.h"

#include "winbase.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSqlModel::QSqlModel(QWidget *pOwner /* = 0 */) : QObject(0)
        , m_pOwner(pOwner)
        , m_qsDBSqliteFile("Uninitialized")
        , m_iDbEngine(DB_Engine_Undefined)
        , m_qsDatabaseName("rmo")
        , m_qsDBUserName("postgres")
        , m_qsDBPassword("123")
        , m_qsDBHost("127.0.0.1")
        , m_qsDBEncoding("WIN1251")
        , m_pQuery(0)
        , m_pRecord(0)
        , m_qsConnectionName(QString()) {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QSQLMODEL_SQLITE_FILE,"poite20170602.db");
    m_qsDBSqliteFile = iniSettings.value(QSQLMODEL_SQLITE_FILE,scRes).toString();
    iniSettings.setDefault(QSQLMODEL_DBENGINE,DB_Engine_Sqlite);
    m_iDbEngine = (QSQLMODEL_DB_ENGINES)iniSettings.value(QSQLMODEL_DBENGINE,scRes).toInt();
    iniSettings.setDefault(QSQLMODEL_DATABASENAME,"rmo");
    m_qsDatabaseName = iniSettings.value(QSQLMODEL_DATABASENAME,scRes).toString();
    iniSettings.setDefault(QSQLMODEL_USERNAME,"postgres");
    m_qsDBUserName = iniSettings.value(QSQLMODEL_USERNAME,scRes).toString();
    iniSettings.setDefault(QSQLMODEL_PASSWORD,"123");
    m_qsDBPassword = iniSettings.value(QSQLMODEL_PASSWORD,scRes).toString();
    iniSettings.setDefault(QSQLMODEL_ENCODING,"WIN1251");
    m_qsDBEncoding = iniSettings.value(QSQLMODEL_ENCODING,scRes).toString();
    iniSettings.setDefault(QSQLMODEL_HOSTNAME,"127.0.0.1");
    m_qsDBHost = iniSettings.value(QSQLMODEL_HOSTNAME,scRes).toString();
    QObject::connect(this,SIGNAL(connStatusChanged(QString)),m_pOwner,SLOT(setStatusMessage(QString)));

    openDB();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSqlModel::~QSqlModel() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(QSQLMODEL_SQLITE_FILE, m_qsDBSqliteFile);
    iniSettings.setValue(QSQLMODEL_DBENGINE, m_iDbEngine);
    iniSettings.setValue(QSQLMODEL_DATABASENAME, m_qsDatabaseName);
    iniSettings.setValue(QSQLMODEL_USERNAME, m_qsDBUserName);
    iniSettings.setValue(QSQLMODEL_PASSWORD, m_qsDBPassword);
    iniSettings.setValue(QSQLMODEL_ENCODING, m_qsDBEncoding);
    iniSettings.setValue(QSQLMODEL_HOSTNAME, m_qsDBHost);

    if (!m_qsConnectionName.isEmpty()) {
        {// close DB connection and free resources carefully
            bool bOpen=false; // do not open DB connection at this time
            QSqlDatabase db = QSqlDatabase::database(m_qsConnectionName,bOpen);
            if (m_pRecord) {
                delete m_pRecord;
                m_pRecord=0;
            }
            if (m_pQuery) {
                delete m_pQuery;
                m_pQuery=0;
            }
            if (db.isOpen()) db.close();
        }
        // free resources
        QSqlDatabase::removeDatabase(m_qsConnectionName);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::openDB() {
    // close existing connection
    if (!m_qsConnectionName.isEmpty()) {
        { // close DB connection and free resources carefully
            bool bOpen=false; // do not open DB connection at this time
            QSqlDatabase db = QSqlDatabase::database(m_qsConnectionName,bOpen);
            if (m_pRecord) {
                delete m_pRecord;
                m_pRecord=0;
            }
            if (m_pQuery) {
                delete m_pQuery;
                m_pQuery=0;
            }
            if (db.isOpen()) db.close();
        }
        // free resources
        QSqlDatabase::removeDatabase(m_qsConnectionName);
    }
    QSqlDatabase db;
    if (m_iDbEngine == DB_Engine_Postgres) {
        m_qsConnectionName = PGSQL_CONN_NAME;
        db = QSqlDatabase::addDatabase("QPSQL",m_qsConnectionName);
        db.setHostName(m_qsDBHost);
        db.setDatabaseName(m_qsDatabaseName);
        db.setUserName(m_qsDBUserName);
        db.setPassword(m_qsDBPassword);
        db.setConnectOptions(QString("client_encoding=%1").arg(m_qsDBEncoding));
    }
    else if (m_iDbEngine == DB_Engine_Sqlite) {
        m_qsConnectionName = SQLITE_CONN_NAME;
        db = QSqlDatabase::addDatabase("QSQLITE",m_qsConnectionName);
        db.setDatabaseName(QFileInfo(m_qsDBSqliteFile).absoluteFilePath());
    }
    // if db open fails then show exception dialog
    if (!db.open()) {
        QString qsDBError(db.lastError().text());
        qDebug() << "Cannot open database:" << qsDBError;
        QExceptionDialog *pDlg = new QExceptionDialog(qsDBError, m_pOwner);
        pDlg -> setAttribute(Qt::WA_DeleteOnClose);
        pDlg -> open();
        connStatusChanged(CONN_STATUS_DISCONN);
        return false;
    }
    // update connection status on the status bar
    if (m_iDbEngine == DB_Engine_Postgres) {
        emit connStatusChanged(CONN_STATUS_PGCONN);
    }
    else if (m_iDbEngine == DB_Engine_Sqlite) {
        emit connStatusChanged(CONN_STATUS_SQLITE);
    }
    // open already succeeded
    return true;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::testPgConnection(QString &qsErrMsg, QString qsHost, QString qsDBName, QString qsUser, QString qsPassword, QString qsEncoding) {
    bool retVal;
    qsErrMsg.clear();
    QString qsConnName("test_postgres_db_connection");
    { // use inner scope for QSqlDatabase db object.
      //  Otherwise: "connection is still in use all queries will cease to work"
        QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL",qsConnName);
        db.setHostName(qsHost);
        db.setDatabaseName(qsDBName);
        db.setUserName(qsUser);
        db.setPassword(qsPassword);
        db.setConnectOptions(QString("client_encoding=%1").arg(qsEncoding));
        retVal = db.open();
        if (retVal) {
            db.close();
        }
        else {
            qsErrMsg = db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(qsConnName);
    return retVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::testSqliteConnection(QString &qsErrMsg, QString qsDBName) {
    bool retVal;
    qsErrMsg.clear();
    QString qsConnName("test_sqlite_db_connection");
    { // use inner scope for QSqlDatabase db object.
      //  Otherwise: "connection is still in use all queries will cease to work"
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",qsConnName);
        db.setDatabaseName(qsDBName);
        retVal = db.open();
        if (retVal) {
            db.close();
        }
        else {
            qsErrMsg = db.lastError().text();
        }
    }
    QSqlDatabase::removeDatabase(qsConnName);
    return retVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pPropPages->m_pbgDBEngine=new QButtonGroup;
    // postgres db connection pareameters
    pPropPages->m_prbDBEnginePostgres=new QRadioButton("PostgreQSL: DB connection parameters");
    pVLayout->addWidget(pPropPages->m_prbDBEnginePostgres);
    QGridLayout *pGridLayout=new QGridLayout;
    pPropPages->m_pbgDBEngine->addButton(pPropPages->m_prbDBEnginePostgres);
    pGridLayout->addWidget(new QLabel("Database name"),0,0);
    pPropPages->m_pleDBDatabaseName=new QLineEdit(m_qsDatabaseName);
    pGridLayout->addWidget(pPropPages->m_pleDBDatabaseName,0,1);
    pGridLayout->addWidget(new QLabel("Database user"),0,3);
    pPropPages->m_pleDBDatabaseUser=new QLineEdit(m_qsDBUserName);
    pGridLayout->addWidget(pPropPages->m_pleDBDatabaseUser,0,4);
    pGridLayout->addWidget(new QLabel("Database password"),1,0);
    pPropPages->m_pleDBDatabasePassword=new QLineEdit(m_qsDBPassword);
    pGridLayout->addWidget(pPropPages->m_pleDBDatabasePassword,1,1);
    pGridLayout->addWidget(new QLabel("Database encoding"),1,3);
    pPropPages->m_pleDBDatabaseEncoding=new QLineEdit(m_qsDBEncoding);
    pGridLayout->addWidget(pPropPages->m_pleDBDatabaseEncoding,1,4);
    pGridLayout->addWidget(new QLabel("Database hostname"),2,0);
    pPropPages->m_pleDBDatabaseHostname=new QLineEdit(m_qsDBHost);
    pGridLayout->addWidget(pPropPages->m_pleDBDatabaseHostname,2,1);
    pGridLayout->addWidget(new QLabel("Postgres connection:"),2,3);
    QPushButton *ppbTestPgConnection = new QPushButton("test");
    pGridLayout->addWidget(ppbTestPgConnection,2,4);
    QObject::connect(ppbTestPgConnection,SIGNAL(clicked()),pPropPages,SIGNAL(testPgConnection()));
    QObject::connect(pPropPages,SIGNAL(testPgConnection()),SLOT(onTestPgConnection()));
    pGridLayout->setColumnStretch(5,100);
    pGridLayout->setColumnStretch(2,100);
    pVLayout->addLayout(pGridLayout);
    // sqlite DB file
    pPropPages->m_prbDBEngineSqlite=new QRadioButton("SQLite: DB file");
    pPropPages->m_pbgDBEngine->addButton(pPropPages->m_prbDBEngineSqlite);
    pVLayout->addWidget(pPropPages->m_prbDBEngineSqlite);
    pPropPages->m_pleDBSqliteFileName=new QLineEdit(m_qsDBSqliteFile);
    QHBoxLayout *pHBoxLayout=new QHBoxLayout;
    pHBoxLayout->addWidget(pPropPages->m_pleDBSqliteFileName);
    QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseSqliteFile()));
    QObject::connect(pPropPages,SIGNAL(chooseSqliteFile()),SLOT(onSQLiteFileChoose()));
    pHBoxLayout->addWidget(ppbChoose);
    pVLayout->addLayout(pHBoxLayout);
    pVLayout->addStretch();
    if (m_iDbEngine == DB_Engine_Postgres) {
        pPropPages->m_prbDBEnginePostgres->setChecked(true);
    }
    else if (m_iDbEngine == DB_Engine_Sqlite) {
        pPropPages->m_prbDBEngineSqlite->setChecked(true);
    }
    else {
        pPropPages->m_prbDBEnginePostgres->setChecked(false);
        pPropPages->m_prbDBEngineSqlite->setChecked(false);
    }
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QSQLMODEL_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);

    QSQLMODEL_DB_ENGINES iDbEngine;
    QString qsDBHost;
    QString qsDatabaseName;
    QString qsDBUserName;
    QString qsDBPassword;
    QString qsDBEncoding;
    QString qsDBSqliteFile;

    // DB engine selection
    if (pPropPages->m_prbDBEnginePostgres->isChecked()) {
        iDbEngine = DB_Engine_Postgres;
        qsDBHost=pPropPages->m_pleDBDatabaseHostname->text();
        qsDatabaseName=pPropPages->m_pleDBDatabaseName->text();
        qsDBUserName=pPropPages->m_pleDBDatabaseUser->text();
        qsDBPassword=pPropPages->m_pleDBDatabasePassword->text();
        qsDBEncoding=pPropPages->m_pleDBDatabaseEncoding->text();
    }
    else if (pPropPages->m_prbDBEngineSqlite->isChecked()) {
        iDbEngine = DB_Engine_Sqlite;
        qsDBSqliteFile=pPropPages->m_pleDBSqliteFileName->text();
    }
    else {
        QString qsDBError("No database engine selected");
        qDebug() << "Cannot open database:" << qsDBError;
        QExceptionDialog *pDlg = new QExceptionDialog(qsDBError, m_pOwner);
        pDlg -> setAttribute(Qt::WA_DeleteOnClose);
        pDlg -> open();
        return;
    }

    // test new connection
    bool bConnChanged=false;
    QString qsErrMsg;
    if (iDbEngine!=m_iDbEngine) bConnChanged = true;
    if (iDbEngine==DB_Engine_Postgres) {
        if (qsDBHost != m_qsDBHost || qsDatabaseName != m_qsDatabaseName
         || qsDBUserName != m_qsDBUserName || qsDBPassword != m_qsDBPassword
         || qsDBEncoding != m_qsDBEncoding) bConnChanged = true;
    }
    if (iDbEngine==DB_Engine_Sqlite) {
        if (qsDBSqliteFile != m_qsDBSqliteFile) bConnChanged = true;
    }

    // only reopen if connection parameters changed
    if (bConnChanged) {
        bool bOk;
        if (iDbEngine==DB_Engine_Sqlite) {
            bOk = testSqliteConnection(qsErrMsg, qsDBSqliteFile);
        }
        if (iDbEngine==DB_Engine_Postgres) {
            bOk = testPgConnection(qsErrMsg, qsDBHost, qsDatabaseName,
                                       qsDBUserName, qsDBPassword, qsDBEncoding);
        }
        if (!bOk) {
            qDebug() << "Cannot open database:" << qsErrMsg;
            QExceptionDialog *pDlg = new QExceptionDialog(qsErrMsg, m_pOwner);
            pDlg -> setAttribute(Qt::WA_DeleteOnClose);
            pDlg -> open();
            return;
        }
        if (iDbEngine==DB_Engine_Sqlite) {
            m_iDbEngine=iDbEngine;
            m_qsDBSqliteFile=qsDBSqliteFile;
            bOk=openDB();
            if (bOk) emit connStatusChanged(CONN_STATUS_SQLITE);
        }
        if (iDbEngine==DB_Engine_Postgres) {
            m_iDbEngine=iDbEngine;
            m_qsDBHost=qsDBHost;
            m_qsDatabaseName=qsDatabaseName;
            m_qsDBUserName=qsDBUserName;
            m_qsDBPassword=qsDBPassword;
            m_qsDBEncoding=qsDBEncoding;
            bOk=openDB();
            if(bOk) emit connStatusChanged(CONN_STATUS_PGCONN);
        }
        if (!bOk) emit connStatusChanged(CONN_STATUS_DISCONN);
        return;
    }

    // otherwise just emit signal about connection status
    if (!m_qsConnectionName.isEmpty()) {
        // get existing connection (must already be opened)
        bool bOpen=false; // do not open DB connection at this time
        QSqlDatabase db = QSqlDatabase::database(m_qsConnectionName,bOpen);
        if (db.isOpen()) {
            if (iDbEngine==DB_Engine_Sqlite) emit connStatusChanged(CONN_STATUS_SQLITE);
            else if (iDbEngine==DB_Engine_Postgres) emit connStatusChanged(CONN_STATUS_PGCONN);
            return;
        }
    }
    emit connStatusChanged(CONN_STATUS_DISCONN);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::checkPoite() {
    QSqlDatabase db;
    // if no connection was set - then skip all
    if (m_qsConnectionName.isEmpty()) return;
    // get existing connection (must already be opened)
    bool bOpen=false; // do not open DB connection at this time
    db = QSqlDatabase::database(m_qsConnectionName,bOpen);
    if (!db.isOpen()) return;
    QString qsQuery("SELECT p.id AS id,"
        " p.codogram"
        " FROM poite p");
    QSqlQuery query(db);
    query.prepare(qsQuery);
    if (!query.exec()) return;
    QSqlRecord qRec=query.record();
    QFile qfTest("poitetest.dat");
    qfTest.open(QIODevice::ReadWrite);
    QTextStream tsTest(&qfTest);
    while (query.next()) {
        QByteArray baCodogram;
        QByteArray baUncomp;
        PPOITE pPoite;

        if (m_iDbEngine==DB_Engine_Sqlite) {
            QString qsCodogram = query.value(qRec.indexOf("codogram")).toString();
            baCodogram = QByteArray(qsCodogram.toLocal8Bit());
            QByteArray baDecoded = QByteArray::fromBase64(baCodogram);
            baUncomp = qUncompress(baDecoded);
            pPoite = (PPOITE)baUncomp.data();
        }
        else if (m_iDbEngine==DB_Engine_Postgres) {
            baCodogram = query.value(qRec.indexOf("codogram")).toByteArray();
            pPoite = (PPOITE)baCodogram.data();
        }
        else return;

        FILETIME ftTlock=pPoite->ftTlock;
        SYSTEMTIME st;
        FileTimeToSystemTime(&ftTlock,&st);
        tsTest << st.wYear << "." << st.wMonth << "." << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::execQuery() {
    QSqlDatabase db;
    // if no connection was set - then skip all
    if (m_qsConnectionName.isEmpty()) return false;
    // get existing connection (must already be opened)
    bool bOpen=false; // do not open DB connection at this time
    db = QSqlDatabase::database(m_qsConnectionName,bOpen);
    if (!db.isOpen()) return false;
    // total records
    // QSqlQuery query = QSqlQuery(db);
    // if (!query.exec("SELECT COUNT (DISTINCT p.id) AS cnt FROM poite p")) return false;
    // if (!query.next()) return false;
    // QSqlRecord rec = query.record();
    // qDebug() << "Total: " << query.value(rec.indexOf("cnt")).toULongLong() << " records";

    // basic query - codograms POITE
    m_pQuery = new QSqlQuery(db);
    if (!m_pQuery) return false;
    QString qsQuery("SELECT p.id AS id,"
        " p.codogram AS cdata"
        " FROM poite p");
    if (!m_pQuery->prepare(qsQuery)) {
        delete m_pQuery;
        m_pQuery=0;
        return false;
    }
    if (!m_pQuery->exec()) {
        delete m_pQuery;
        m_pQuery=0;
        return false;
    }
    m_pRecord = new QSqlRecord();
    *m_pRecord = m_pQuery->record();
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::getTuple(quint64 &iRecId, QByteArray &baCodogram) {
    bool bOk;
    if (!m_pQuery || !m_pQuery->isActive()) {
        qDebug() << "!m_pQuery.isActive()";
        return false;
    }
    if (!m_pQuery->next()) {
        return false;
    }
    if (!m_pRecord) {
        return false;
    }
    iRecId=m_pQuery->value(m_pRecord->indexOf("id")).toULongLong(&bOk);
    if (!bOk) {
        qDebug() << "m_pQuery->value(m_pRecord->indexOf(id)).toULongLong(&bOk) failed!";
        return false;
    }
    baCodogram=m_pQuery->value(m_pRecord->indexOf("cdata")).toByteArray();
    if (baCodogram.isEmpty()) {
        qDebug() << "baCodogram.isEmpty()!";
        return false;
    }
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::onSQLiteFileChoose() {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());

    QFileDialog dialog(pPropPages);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("SQLite db file (*.db *.sqlite *.sqlite3)"));
    dialog.setDirectory(QDir::current());
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable())	{
            QDir qdCur = QDir::current();
            QDir qdSelFilePath = fiSelFilePath.absoluteDir();
            if (qdCur.rootPath() == qdSelFilePath.rootPath()) { // same Windows drives
                pPropPages->m_pleDBSqliteFileName->setText(qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath()));
            }
            else { // different Windows drives
                pPropPages->m_pleDBSqliteFileName->setText(fiSelFilePath.absoluteFilePath());
            }
        }
        else {
            pPropPages->m_pleDBSqliteFileName->setText("error");
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::onTestPgConnection() {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());
    if (!pPropPages) {
        emit connStatusChanged("Could not start test");
        return;
    }

    QString qsErrMsg;
    bool bOk = testPgConnection(
                qsErrMsg,
                pPropPages->m_pleDBDatabaseHostname->text(),
                pPropPages->m_pleDBDatabaseName->text(),
                pPropPages->m_pleDBDatabaseUser->text(),
                pPropPages->m_pleDBDatabasePassword->text(),
                pPropPages->m_pleDBDatabaseEncoding->text());
    // set status bar message depending on result
    if (bOk) {
        emit connStatusChanged("Postgres test succeeded");
    }
    else {
        emit connStatusChanged(qsErrMsg);
    }
}
