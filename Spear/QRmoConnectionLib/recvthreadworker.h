#ifndef RECVTHREADWORKER_H
#define RECVTHREADWORKER_H

#include "qrmoconnection.h"
#include "qrmoconnection_p.h"

class ConnectionThreadWorker;

//-----------------------------------------------------------------------
// class used for receiving data over network
//-----------------------------------------------------------------------
class RecvThreadWorker : public QObject {
    Q_OBJECT

public:
    RecvThreadWorker(QRmoConnectionPrivate *pRmoConnectionPrivate);
    ~RecvThreadWorker();
    void cleanup();

signals:
    void recvOk();
    void cleanupNonBlocking();

public slots:
	void onDoRecv(QByteArray *pbaPayload, int iType);
    void onCleanup();

public:
	ConnectionThreadWorker * m_pConnWorker;

private:
    QRmoConnectionPrivate * m_pRmoConnectionPrivate;
};

#endif // RECVTHREADWORKER_H
