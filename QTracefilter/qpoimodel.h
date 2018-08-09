#ifndef QPOIMODEL_H
#define QPOIMODEL_H
#include <QObject>
#include <QMap>
#include <QSqlDatabase>

#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>

#include "qavtctrl.h"
#include "codograms.h"

#include "tpoit.h"

#define SQLITE_CONN_NAME "sqlite_db_connection"

class QPoiModel {

public:
	enum POI_DB_ENGINES {
		DB_Engine_Undefined,
		DB_Engine_Sqlite,
		DB_Engine_Postgres
	};

	QPoiModel();
	~QPoiModel();

	bool initPostgresDB(QString qsPgConnStr);
	bool initSqliteDB(QFile &qfSqlite);
    bool readPoite(quint64 tFrom, quint64 tTo, double dHeight);

	void clearTPoiTList();
    bool getMinMaxTime(quint64 &tFrom,quint64 &tTo);
	static PPOSTCORD m_pPostCord;

	int m_nSol;

    const double BETA_TOLERANCE;
    const double RMAXIMUM;

	double m_dRmax;

    QList<int> buildSlidingWindow(quint64 tFrom, quint64 tTo);
    quint64 getPoiteTime(int iSlWindowIdx);
    TPoiT* getTPoiT(int iSlWindowIdx);
	void releaseDataSource();

private:
	bool openDB();
    void closeDB();

	QSqlDatabase m_db;
	PGconn *m_conn;
	PGresult *m_res;
	QList<TPoiT*> m_qlPTPoiT;
	QList<quint64> m_qlPoiteTime;
	POI_DB_ENGINES m_iDbEngine;
    QString m_qsPgConnStr;
};

#endif // QPOIMODEL_H
