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

class QIndicator;
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

    void clearRawLists();
    bool getMinMaxTime(quint64 &tFrom,quint64 &tTo);
	static PPOSTCORD m_pPostCord;

	int m_nSol;

    const double BETA_TOLERANCE;
    const double RMAXIMUM;

	double m_dRmax;

    QList<int> buildPrunedList(quint64 tFrom, quint64 tTo);
    quint64 getPoiteTime(int iRawIdx);
    TPoiT* getTPoiT(int iRawIdx);
    QByteArray getPPoite(int iRawIdx);
    int getRawListSize() {
        int nPoite=m_qlPPoite.size();
        int nTPoiT=m_qlPTPoiT.size();
        int nPoiteTime=m_qlPoiteTime.size();
        if (nPoiteTime==nPoite && nPoiteTime==nTPoiT) return nPoiteTime;
        return -1;
    }

	void releaseDataSource();

private:
	bool openDB();
    void closeDB();

	QSqlDatabase m_db;
	PGconn *m_conn;
	PGresult *m_res;
	QList<TPoiT*> m_qlPTPoiT;
    QList<QByteArray> m_qlPPoite;
    QList<quint64> m_qlPoiteTime;
	POI_DB_ENGINES m_iDbEngine;
    QString m_qsPgConnStr;
    friend class QIndicator;
};

#endif // QPOIMODEL_H
