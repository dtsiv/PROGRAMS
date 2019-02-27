#ifndef SENDTHREADWORKER_H
#define SENDTHREADWORKER_H

#include "qrmoconnection.h"
#include "qrmoconnection_p.h"

#define SENDTHREADWORKER_MAX_THREADS_PER_WAIT_CONDITION 2

class ConnectionThreadWorker;

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
struct SendRequestInfo {
	QWaitCondition waitConditionNextEnabled;
	QWaitCondition waitConditionSendComplete;
};

//-----------------------------------------------------------------------
// class used for sending data over network
//-----------------------------------------------------------------------
class SendThreadWorker : public QObject {
    Q_OBJECT

public:
    SendThreadWorker(QRmoConnectionPrivate *pRmoConnectionPrivate);
    ~SendThreadWorker();

	bool lockSend(
		QByteArray *pbaPayload, 
		int iType, 
		unsigned long timeout = ULONG_MAX);
    void raiseError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);
	void afterConnected();

signals:
	void doSend(QByteArray *pbaPayload, int iType);
    void error(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);
	void clearError();

public slots:
    void onError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);
	void onClearError();
	void unlockSend();

public:
	ConnectionThreadWorker * m_pConnWorker;
	QAbstractSocket::SocketError m_iErrorCode;
	QString m_qsErrorString;

private:
	QMutex m_mutex;
    QQueue<SendRequestInfo*> m_senders;
	bool m_bError;

private:
	QRmoConnectionPrivate * m_pRmoConnectionPrivate;
};

#endif // SENDTHREADWORKER_H
