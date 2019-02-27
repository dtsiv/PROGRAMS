#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>
#include <QtGlobal>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <math.h>
#include <stdlib.h>

#include "qrmoconnection.h"

class Receiver : public QObject
{
	Q_OBJECT

public:
	Receiver(QObject *parent
		, QRmoConnection * pConnection
		, int iDelay);

	~Receiver();

public slots:
	void onReceive(QByteArray*,int);

private:
	QRmoConnection * m_pConnection;
	quint64 m_nDelayMs;
	bool m_bStop;
    QMutex m_mutex;
	QWaitCondition m_condition;

public:
    quint64 m_iSize;
    quint64 m_iTotalSize;

	void wake();
};

#endif // RECEIVER_H
