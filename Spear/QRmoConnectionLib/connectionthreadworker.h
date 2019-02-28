#ifndef CONNECTIONTHREADWORKER_H
#define CONNECTIONTHREADWORKER_H

#include "qrmoconnection.h"
#include "qrmoconnection_p.h"
#include "sendthreadworker.h"
#include "recvthreadworker.h"

//-----------------------------------------------------------------------
// class used for network connection 
//-----------------------------------------------------------------------
class ConnectionThreadWorker : public QObject {
    Q_OBJECT

public:
    ConnectionThreadWorker(QRmoConnectionPrivate *pRmoConnectionPrivate, QThread * pConnectionThread);
    ~ConnectionThreadWorker();

    void closeConnection();
	bool isConnected();
	void cleanup();
	void reconnect();
    void startTransfer();

private:
	void readPayload();
	void setSocketOptions(QTcpSocket *pSocket);
    void connectSlotsToSocket(QTcpSocket *pSocket);
    void startListen();

public slots:
    void tryConnect();
    void onStarted();
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);
    void onDoSend(QByteArray *, int);
	void onRecvOk();
	void onBytesWritten(qint64);
    void onCleanup();
	void onNewConnection();
	// this slot is called directly from send thread
	void onReadyForTransfer();
    void onDoReconnect();
    void onDoDisconnect();

signals:
    void cleanupNonBlocking();
	void sendOk();
	void doRecv(QByteArray*,int);
	void connectedStateChanged(QString,int,bool);
	void doReconnect();
    void doDisconnect();
    void readyForTransfer();

private:
    QRmoConnectionPrivate * m_pRmoConnectionPrivate;

public:
	SendThreadWorker * m_pSendThreadWorker;
    RecvThreadWorker * m_pRecvThreadWorker;

private:
    quint64	 m_nNextBlockSize;
    QTcpSocket *m_pTcpSocket;
	QTcpServer *m_pTcpServer;
    QTimer *m_pTimer;
    bool m_bStop;
	bool m_bConnected;
    ConnNS::ProtocolHeader m_receiveHeader;
    ConnNS::ProtocolHeader m_sendHeader;
    bool m_bIsNextBlockHeader;
    QByteArray *m_pbaPartialBuffer;
    qint64 m_iTotalBytesRead;
};

#endif // CONNECTIONTHREADWORKER_H
