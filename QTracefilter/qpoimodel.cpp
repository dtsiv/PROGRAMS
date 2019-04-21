#include "qpoimodel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include "qavtctrl.h"
#include "codograms.h"

#include "tpoit.h"
#include "rmoexception.h"

// quint64 uGlobalId;
bool TdCord::bUseRxOffsets=false;
double TdCord::dRxOffsets[5] = { 0.,  0.,  0.,  0.,  0. };

PPOSTCORD QPoiModel::m_pPostCord = NULL;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPoiModel::QPoiModel()
	: BETA_TOLERANCE(1.0e-3)
	, RMAXIMUM(TdCord::dRMaxinun)
	, m_dRmax(RMAXIMUM)
    , m_iDbEngine(DB_Engine_Undefined)
    , m_conn(NULL)
    , m_res(NULL) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPoiModel::~QPoiModel() {
	// PQfinish(m_conn);
	// m_conn=0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoiModel::initPostgresDB(QString qsPgConnStr) {
	char * conninfo;

    QByteArray baPgConnStr=qsPgConnStr.toLocal8Bit();
    conninfo = (char *)baPgConnStr.data();
    /* Make a connection to the database:
	*  pgclient "dbname=rmo user=postgres password=123 client_encoding=WIN1251"
	*/
    m_conn = PQconnectdb(conninfo);
	/* Check to see that the backend connection was successfully made */
	if (PQstatus(m_conn) != CONNECTION_OK) {
		QString qsDBError(PQerrorMessage(m_conn));
        qDebug() << "Connection to database failed: " << qsDBError;
		throw RmoException(QString("PostgreSQL database error: ")+qsDBError);
		return false;
	}
	m_qsPgConnStr = qsPgConnStr;
	m_iDbEngine = QPoiModel::DB_Engine_Postgres;
    PQfinish(m_conn);
	m_conn=0;
	return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoiModel::initSqliteDB(QFile &qfSqlite) {
	if (m_db.isValid()) {
		QString qsConnName=m_db.connectionName();
		m_db = QSqlDatabase();
		QSqlDatabase::removeDatabase(qsConnName);
	}
    m_db = QSqlDatabase::addDatabase("QSQLITE",SQLITE_CONN_NAME);
	m_db.setDatabaseName(QFileInfo(qfSqlite).absoluteFilePath());
    m_db.setUserName("user"); 
    m_db.setHostName("localhost"); 
    m_db.setPassword("password"); 
    if (!m_db.open()) {
		QString qsDBError(m_db.lastError().text());
        qDebug() << "Cannot open database:" << qsDBError;
		throw RmoException(QString("SQLite database error: ")+qsDBError);
        return false;
    }
	m_iDbEngine = QPoiModel::DB_Engine_Sqlite;
	m_db.close();
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoiModel::openDB() {
	if (m_iDbEngine == QPoiModel::DB_Engine_Sqlite) {
		if (!m_db.open()) {
			QString qsDBError(m_db.lastError().text());
			qDebug() << "Cannot open database:" << qsDBError;
			throw RmoException(QString("SQLite database error: ")+qsDBError);
			return false;
		}
	    return true;
	}
	else if (m_iDbEngine == QPoiModel::DB_Engine_Postgres) {
		char * conninfo;
        QByteArray baPgConnStr = m_qsPgConnStr.toLocal8Bit();
        conninfo = (char *)baPgConnStr.data();
		m_conn = PQconnectdb(conninfo);

		/* Check to see that the backend connection was successfully made */
		if (PQstatus(m_conn) != CONNECTION_OK) {
			QString qsDBError(PQerrorMessage(m_conn));
            qDebug() << "Connection to database failed: " << qsDBError;
			throw RmoException(QString("PostgreSQL database error: ")+qsDBError);
			return false;
		}
	    return true;
	}
	else {
		qDebug() << "m_iDbEngine = " << m_iDbEngine;
		return false;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoiModel::releaseDataSource() {
	if (m_iDbEngine == QPoiModel::DB_Engine_Sqlite) {
		if (m_db.isValid()) m_db.close();
		QString qsConnName=m_db.connectionName();
		if (qsConnName==SQLITE_CONN_NAME) {
			m_db = QSqlDatabase();
			// qDebug() << "Removing db " << qsConnName;
			QSqlDatabase::removeDatabase(qsConnName);
		}
	}
	else if (m_iDbEngine == QPoiModel::DB_Engine_Postgres) {
		if (m_conn) PQfinish(m_conn);
		m_conn=0;
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoiModel::closeDB() {
	if (m_iDbEngine == QPoiModel::DB_Engine_Sqlite) {
		m_db.close();
	}
	else if (m_iDbEngine == QPoiModel::DB_Engine_Postgres) {
		if (m_conn) {
			PQfinish(m_conn);
			m_conn=0;
		}
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoiModel::getMinMaxTime(quint64 &tFrom,quint64 &tTo) {
	int i;
	if (m_iDbEngine == QPoiModel::DB_Engine_Postgres) {
		if (!openDB()) {
			QString qsPostgresError(PQerrorMessage(this->m_conn));
			qDebug() << "PostgreSQL database error: " << qsPostgresError;
			throw RmoException(qsPostgresError);
			return false;
		}
		// time range query
        char *sTimeQuery = (char *)"SELECT MIN(p.time_from) AS min_time, "
						   " MAX(p.time_from) AS max_time FROM poite p";
		m_res = PQexecParams(m_conn,sTimeQuery,
			0, /* 0 param */
			NULL, /* let the backend deduce param type */
			NULL, // no input params
			NULL, /* don’t need param lengths since text */
			NULL, /* default to all text params */
			0); /* ask for binary results */
		if (PQresultStatus(m_res) != PGRES_TUPLES_OK || PQntuples(m_res) != 1) {
			qDebug() << "SELECT failed: " <<  PQerrorMessage(m_conn);
			PQclear(m_res);
			return false;
		}
		int imin = PQfnumber(m_res, "min_time");
		int imax = PQfnumber(m_res, "max_time");
		char *pmin;
		char *pmax;
		/* Get the field values (we ignore possibility they are null!) */
		pmin = PQgetvalue(m_res, 0, imin);
		pmax = PQgetvalue(m_res, 0, imax);

		bool bOk1,bOk2;
		tFrom = QString(pmin).toULongLong(&bOk1);
		tTo = QString(pmax).toULongLong(&bOk2);

		PQclear(m_res);
		PQfinish(m_conn);
		m_conn=0;
		return (bOk1 && bOk2);
	}
	else if (m_iDbEngine == QPoiModel::DB_Engine_Sqlite) {
		if (!openDB()) {
			QString qsSqliteError(m_db.lastError().text());
			qDebug() << "Sqlite error: " << qsSqliteError;
			throw RmoException(QString("Sqlite error: ") + qsSqliteError);
			return false;
		}
		// time range query
		QString qsTimeQuery = "SELECT MIN(p.time_from) AS min_time, "
						   " MAX(p.time_from) AS max_time FROM poite p";
		QSqlQuery query(m_db);
		query.prepare(qsTimeQuery);
		if (!query.exec() || !query.next()) {
			QString qsSqliteError(query.lastError().text());
			qDebug() << "Time query failed: " << qsSqliteError;
			throw RmoException(QString("Sqlite error: ") + qsSqliteError);
			m_db.rollback(); m_db.close();
		}
		QSqlRecord qRec=query.record();
		bool bOk1,bOk2;
		tFrom = query.value(qRec.indexOf("min_time")).toULongLong(&bOk1);
		tTo = query.value(qRec.indexOf("max_time")).toULongLong(&bOk2);
		query.clear();
        closeDB();
		return (bOk1 && bOk2);
	}
	qDebug() << "m_iDbEngine =" << m_iDbEngine;
	return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QPoiModel::readPoite(quint64 tFrom, quint64 tTo, double dHeight) {
	int i;
	if (m_iDbEngine == QPoiModel::DB_Engine_Postgres) {
		const char *paramValues[2];
		int paramLengths[2];
		int paramFormats[2];

		if (!openDB()) {
			QString qsPostgresError(PQerrorMessage(this->m_conn));
			qDebug() << "PostgreSQL error: " << qsPostgresError;
			throw RmoException(QString("PostgreSQL databse error: ") + qsPostgresError);
			return false;
		}

		// Start a transaction block
		m_res = PQexec(m_conn, "BEGIN");
		if (PQresultStatus(m_res) != PGRES_COMMAND_OK) {
			qDebug() << "BEGIN command failed: " << PQerrorMessage(m_conn);
			PQclear(m_res);
			return false;
		}
		PQclear(m_res);
		QByteArray baFrom=QString::number(tFrom).toLocal8Bit();
		QByteArray baTo=QString::number(tTo).toLocal8Bit();
		paramValues[0] = baFrom.data();
		paramLengths[0] = NULL;
		paramFormats[0] = 0; /* text */
		paramValues[1] = baTo.data();
		paramLengths[1] = NULL;
		paramFormats[1] = 0; /* text */

		QString qsQuery("SELECT p.id ::character varying(255) AS id,"
			" p.time_from::character varying(255) AS time_from,"
			" p.codogram,"
			" p.count::character varying(255) AS \"count\""
			" FROM poite p WHERE p.time_from >= $1"
			" AND p.time_from <= $2"
			" AND (p.count>1 OR p.count IS NULL)");

		m_res = PQexecParams(m_conn,
			qsQuery.toLocal8Bit().data(),
			2, /* 2 param */
			NULL, /* let the backend deduce param type */
			paramValues,
			NULL, /* don’t need param lengths since text */
			NULL, /* default to all text params */
			1); /* ask for binary results */
		if (PQresultStatus(m_res) != PGRES_TUPLES_OK) {
			fprintf(stderr, "SELECT failed: %s", PQerrorMessage(m_conn));
			PQclear(m_res);
			return false;
		}
		/* Use PQfnumber to avoid assumptions about field order in result */
		int idnum = PQfnumber(m_res, "id");
		int tnum = PQfnumber(m_res, "time_from");
		int cnum = PQfnumber(m_res, "codogram");
		int cntnum = PQfnumber(m_res, "count");

		for (i = 0; i < PQntuples(m_res); i++) {
			char *idptr;
			char *timeptr;
			char *cptr;
			char *cntptr;
			int clen;
			/* Get the field values (we ignore possibility they are null!) */
			idptr = PQgetvalue(m_res, i, idnum);
			timeptr = PQgetvalue(m_res, i, tnum);
			cptr = PQgetvalue(m_res, i, cnum);
			cntptr = PQgetvalue(m_res, i, cntnum);
			clen = PQgetlength(m_res, i, cnum);

			bool bOk;
			quint64 tCodogram = QString(timeptr).toULongLong(&bOk);
			if (!bOk) continue;
			PPOITE pPoite = (PPOITE)cptr;

			TPoiT *pTPoiT = new TPoiT(pPoite);
			pTPoiT->m_dHeight=dHeight;
			m_qlPTPoiT.append(pTPoiT);

			m_qlPoiteTime.append(tCodogram);
		}
		PQclear(m_res);

		// End a transaction block
		m_res = PQexec(m_conn, "END");
		PQclear(m_res);
		PQfinish(m_conn);
		m_conn=0;
	    return true;
    }
	else if (m_iDbEngine == QPoiModel::DB_Engine_Sqlite) {
		if (!openDB()) {
			qDebug() << "Sqlite error: " << m_db.lastError().text();
			return false;
		}
		// time range query
		QString qsQuery("SELECT p.id AS id,"
			" p.time_from AS time_from,"
			" p.codogram,"
			" p.count AS \"count\""
			" FROM poite p WHERE p.time_from >= :tFrom"
			" AND p.time_from <= :tTo");
		QSqlQuery query(m_db);
		query.prepare(qsQuery);
		query.bindValue(":tFrom",tFrom);
		query.bindValue(":tTo",tTo);
		if (!query.exec()) {
			qDebug() << "Time query failed: " << query.lastError().text();
			m_db.rollback(); m_db.close();
		}
	    QSqlRecord qRec=query.record();
		while (query.next()) {
			bool bOk;
			quint64 tCodogram = query.value(qRec.indexOf("time_from")).toULongLong(&bOk);
			if (!bOk) continue;

			QString qsCodogram = query.value(qRec.indexOf("codogram")).toString();
			if (!bOk) continue;

            QByteArray baCodogram(qsCodogram.toLocal8Bit());
			QByteArray baDecoded = QByteArray::fromBase64(baCodogram);
			QByteArray baUncomp = qUncompress(baDecoded);
			if (baUncomp.isEmpty()) {
				throw RmoException("QPoiModel qUncompress failed!");
				continue;
			}
			PPOITE pPoite = (PPOITE)baUncomp.data();
			if (pPoite->Count > 3 || pPoite->Count < 0) {
				throw RmoException("QPoiModel pPoite->Count not in {1,2,3}");
				continue;
			}

			TPoiT *pTPoiT = new TPoiT(pPoite);
			pTPoiT->m_dHeight=dHeight;
			m_qlPTPoiT.append(pTPoiT);

			m_qlPoiteTime.append(tCodogram);
		}
		query.clear(); // release db file handle
	}
	return false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    clear lists m_qlPTPoiT and m_qlPoiteTime
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoiModel::clearTPoiTList() {
	while(m_qlPTPoiT.count()) delete m_qlPTPoiT.takeFirst();
	m_qlPoiteTime.clear();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    build list for sliding window and solve 2D equation according to bBasic2DSolver
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QList<int> QPoiModel::buildSlidingWindow(quint64 tFrom, quint64 tTo) {
    QList<int> qlRetVal;

	for (int i=0; i<m_qlPoiteTime.count(); i++) {
		quint64 uPoiteTime = m_qlPoiteTime.at(i);
		if (uPoiteTime <=tTo && uPoiteTime >= tFrom) {
            TPoiT* pTPoiT = getTPoiT(i);
			if (pTPoiT == NULL) continue;
			if (QPsVoi::m_UseTolerance) {
			    if (!pTPoiT->CalculateXY()) continue;
			}
			else { // improved 2D solver
			    if (!pTPoiT->CalculateXY_baseSelection()) continue;
				pTPoiT->m_pt.dX = pTPoiT->m_x[0];
				pTPoiT->m_pt.dY = pTPoiT->m_y[0];
			}
			qlRetVal.append(i);
		}
	}
	return qlRetVal;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// extract from private m_qlPoiteTime
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
quint64 QPoiModel::getPoiteTime(int iSlWindowIdx) {
	if (iSlWindowIdx >= 0 && iSlWindowIdx < m_qlPoiteTime.count()) {
        return m_qlPoiteTime.at(iSlWindowIdx);
	}
	// on error return 0
	return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// extract from private m_qlPTPoiT
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPoiT* QPoiModel::getTPoiT(int iSlWindowIdx) {
	if (iSlWindowIdx >= 0 && iSlWindowIdx < m_qlPTPoiT.count()) {
		return m_qlPTPoiT.at(iSlWindowIdx);
	}
	// on error return NULL
	return NULL;
}

