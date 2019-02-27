#ifndef SENDER_H
#define SENDER_H

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QMutex>
#include <QDateTime>
#include <QMutexLocker>
#include <QWaitCondition>

#include <math.h>
#include <stdlib.h>

#include "qrmoconnection.h"

class Sender : public QObject
{
	Q_OBJECT

public:
	Sender(QObject *parent
		, QRmoConnection * pConnection
		, int iBlockSize
		, double dSpeed
		, int iType
		);
	~Sender();
	void finish();

signals:
	void timeout();
	void doFinish();
	void bytesSent(quint64 nBytes);

public slots:
	void onTimeout();
    void onStarted();
	void onDoFinish();

private:
	QRmoConnection * m_pConnection;
	QTimer * m_pTimer;
	qint64 m_iBlockSizeBytes;
	int m_iType;
	int m_iIntervalMsec;
	bool m_bStop;
	QByteArray * m_pbaBuf;
	QTime m_tLast;
	qint64 m_iSent;
public:
	qint64 m_iSent_total;
private:
	quint64 m_nBufSeqNum;
};

#endif // SENDER_H
