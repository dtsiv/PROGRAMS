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
	Receiver(QObject *parent, int iDelay);
	~Receiver();
    void finish();

signals:
    void doFinish();

public slots:
	void onReceive(QByteArray*,int);
    void onDoFinish();

private:
	quint64 m_nDelayMs;
	bool m_bStop;
    QMutex m_mutex;
	QWaitCondition m_condition;

public:
    quint64 m_iSize;
    quint64 m_iTotalSize;
};

#endif // RECEIVER_H
