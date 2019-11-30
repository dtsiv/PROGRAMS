#include "qsqlmodel.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

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
        , m_qsDBEncoding("WIN1251") {
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

    // close postgres DB connection
    m_db.close();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QSqlModel::openDB() {
    if (m_db.isValid()) {
        QString qsConnName=m_db.connectionName();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(qsConnName);
    }
    if (m_iDbEngine == DB_Engine_Postgres) {
        m_db = QSqlDatabase::addDatabase("QPSQL",PGSQL_CONN_NAME);
        m_db.setHostName(m_qsDBHost);
        m_db.setDatabaseName(m_qsDatabaseName);
        m_db.setUserName(m_qsDBUserName);
        m_db.setPassword(m_qsDBPassword);
        m_db.setConnectOptions(QString("client_encoding=%1").arg(m_qsDBEncoding));
    }
    else if (m_iDbEngine == DB_Engine_Sqlite) {
        m_db = QSqlDatabase::addDatabase("QSQLITE",SQLITE_CONN_NAME);
        m_db.setDatabaseName(QFileInfo(m_qsDBSqliteFile).absoluteFilePath());
    }
    // if db open fails then show exception dialog
    if (!m_db.open()) {
        QString qsDBError(m_db.lastError().text());
        qDebug() << "Cannot open database:" << qsDBError;
        QExceptionDialog *pDlg = new QExceptionDialog(qsDBError, m_pOwner);
        pDlg -> setAttribute(Qt::WA_DeleteOnClose);
        pDlg -> open();
        connStatusChanged(CONN_STATUS_DISCONN);
        return false;
    }
    // update connection status on the status bar
    if (m_iDbEngine == DB_Engine_Postgres)    emit connStatusChanged(CONN_STATUS_PGCONN);
    else if (m_iDbEngine == DB_Engine_Sqlite) emit connStatusChanged(CONN_STATUS_SQLITE);
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
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SLOT(onSQLiteFileChoose()));
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
    if (m_db.isOpen()) {
        if (iDbEngine==DB_Engine_Sqlite) emit connStatusChanged(CONN_STATUS_SQLITE);
        else if (iDbEngine==DB_Engine_Postgres) emit connStatusChanged(CONN_STATUS_PGCONN);
        return;
    }
    emit connStatusChanged(CONN_STATUS_DISCONN);
}
