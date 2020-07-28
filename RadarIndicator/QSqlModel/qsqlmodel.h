#ifndef QSQLMODEL_H
#define QSQLMODEL_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>

#include "qinisettings.h"
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
    bool getStrobRecord(quint64 &iRecId, int &iStrob, int &iBeamCountsNum, qint64 &iTimestamp,
                        QByteArray &baStructStrobeData, quint32 &uFileVer);
    bool getBeamData(quint64 &iRecId, int &iBeam, QByteArray &baSamples);
    bool changeDatabaseName(QString qsDbFileName);
    void startTransaction();
    void commitTransaction();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);
    qint64 addFileRec(quint64 uTimeStamp, QString qsFilePath, int nStrobs, QString qsFileVer);
    qint64 addStrobe(int strobeNo, int beamCountsNum, QByteArray baStructStrobeData, qint64 iFileId);
    int addSamples(qint64 iStrobId, int iBeam, char *pSamples, int iSize);

    int dropTables();
    int createTables();
    bool isDBOpen();
    QString getDBFileName() { return m_qsDBFile; }
    void closeDatabase();
    bool getTotStrobes(quint32 &iTotStrobes);

public slots:
    void onSQLiteFileChoose();

private:
	QString m_qsDBFile;

    QSqlQuery m_query;
    QSqlRecord m_record;
};


#endif // QSQLMODEL_H
