#ifndef QSQLMODEL_H
#define QSQLMODEL_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>

#include "qproppages.h"

#define QSQLMODEL_SQLITE_FILE                    "SQLiteFile"
#define QSQLMODEL_DBENGINE                       "DBEngine"
#define QSQLMODEL_DATABASENAME                   "DBDatabaseName"
#define QSQLMODEL_USERNAME                       "DBUserName"
#define QSQLMODEL_PASSWORD                       "DBPassword"
#define QSQLMODEL_ENCODING                       "DBClientEncoding"
#define QSQLMODEL_HOSTNAME                       "DBHostName"


#define DATA_BASE_VERSION            "20191012"

#define QSQLMODEL_PROP_TAB_CAPTION "DB connection"


#define CONN_STATUS_SQLITE   "Connected to SQLite"
#define CONN_STATUS_PGCONN   "Connected to Postgres"
#define CONN_STATUS_DISCONN  "Disconnected from DB"

#define SQLITE_CONN_NAME "sqlite_db_connection"
#define PGSQL_CONN_NAME  "postgres_db_connection"

class QSqlModel : public QObject {
    Q_OBJECT

public:
    enum QSQLMODEL_DB_ENGINES {
        DB_Engine_Undefined,
        DB_Engine_Sqlite,
        DB_Engine_Postgres
    };

    QSqlModel(QWidget *parent = 0);
    ~QSqlModel();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

    bool testPgConnection(QString &qsErrMsg, QString qsHost, QString qsDBName, QString qsUser, QString qsPassword, QString qsEncoding);
    bool testSqliteConnection(QString &qsErrMsg, QString qsDBName);

    void checkPoite();
    bool execQuery();
    bool getTuple(quint64 &iRecId, QByteArray &baCodogram);

signals:
    void connStatusChanged(QString qsStatus);

private:    // database operations
    bool openDB();

private:
    QString m_qsDBSqliteFile;
    QSQLMODEL_DB_ENGINES m_iDbEngine;
    QWidget *m_pOwner;
    QSqlDatabase m_db;
    QString m_qsDatabaseName;
    QString m_qsDBUserName;
    QString m_qsDBPassword;
    QString m_qsDBEncoding;
    QString m_qsDBHost;
    QSqlQuery m_query;
    QSqlRecord m_record;
};

#endif // QSQLMODEL_H
