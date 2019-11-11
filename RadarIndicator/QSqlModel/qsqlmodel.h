#ifndef QSQLMODEL_H
#define QSQLMODEL_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>

#include "qproppages.h"

#define QSQLMODEL_SQLITE_FILE                    "SQLiteFile"
#define DATA_BASE_VERSION           "20191012"

#define QSQLMODEL_PROP_TAB_CAPTION "DB connection"

#define CONN_STATUS_SQLITE   "Connected to SQLite"
#define CONN_STATUS_DISCONN  "Disconnected from DB"

class QSqlModel : public QObject {
	Q_OBJECT

public:
    QSqlModel();
	~QSqlModel();

    bool openDatabase();
    bool execQuery();
    bool getStrobRecord(quint64 &iRecId, int &iStrob, int &iBeamCountsNum, qint64 &iTimestamp);
    bool getBeamData(quint64 &iRecId, int &iBeam, QByteArray &baSamples);

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

private:
	QString m_qsDBFile;
    void closeDatabase();
    QSqlQuery m_query;
    QSqlRecord m_record;
};


#endif // QSQLMODEL_H
