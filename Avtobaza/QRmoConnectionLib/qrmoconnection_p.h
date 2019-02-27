#ifndef QRMOCONNECTION_P_H
#define QRMOCONNECTION_P_H

#include <QtCore>
#include <QQueue>
#include <QWaitCondition>
#include <QMetaObject>
#include <QMetaType>
#include <QThread>
#include <QtNetwork>
#include <QTcpSocket>
#include "qrmoconnection.h"

#include "rmoexception.h"

#define QRMO_CONNECTION_WAIT_TIMEOUT_MSEC             30000
#define QRMO_CONNECTION_POLLING_TIMEOUT_MSEC          3000
#define QRMO_CONNECTION_READ_BUFFER_SIZE              16384
#define QRMO_CONNECTION_OPTION_KEEP_ALIVE             1
#define QRMO_CONNECTION_OPTION_LOW_DELAY              0

#define ERROR_CONTROL_LEVEL_DEBUG

class QRmoConnectionException : public RmoException {
public: 
    QRmoConnectionException(QString str) :
        RmoException(QString("QRmoConnection: " + str)) { }
};

namespace ConnNS {
    typedef union {
        struct {
            int iSize;
            int iType;
            unsigned int dwCS;
        };
        char cdata[12];
    } ProtocolHeader;
}

class ConnectionThreadWorker;
class SendThreadWorker;
class RecvThreadWorker;

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
class QRmoConnectionPrivate
{
    Q_DECLARE_PUBLIC(QRmoConnection)

public:
    QRmoConnectionPrivate(void) { }

    QString m_sHost;
    int m_iPort;
	bool m_bDebugProto;
	QRmoConnection::TcpConnectionRoles m_role;
    void dispatchPayload(QByteArray *pbaPayload, int iType);
	void connectedStateChanged(QString sHost, int iPort, bool bConnected);
	void signalError(QAbstractSocket::SocketError iErrorCode,QString qsErrorString);
	void wakeOnDisconnected();
	QMutex * getWaitForDisconnectedMutex();

private:
    QThread * m_pConnectionThread;
    ConnectionThreadWorker * m_pConnectionThreadWorker;
    QThread * m_pSendThread;
    SendThreadWorker * m_pSendThreadWorker;
    QThread * m_pRecvThread;
    RecvThreadWorker * m_pRecvThreadWorker;
	QMutex m_waitForDisconnectedMutex;
	QWaitCondition m_waitForDisconnectedCondition;
	// init() called from each overloaded constructor
	void init(QString sHost,int iPort,QRmoConnection::TcpConnectionRoles connectionRole, 
	          bool bDebugProto,QObject *parent);
	// currently, startThreads() is called from constructor
    void startThreads();

private:
    QRmoConnection * q_ptr;
};

Q_DECLARE_METATYPE(QAbstractSocket::SocketError)

#endif // QRMOCONNECTION_P_H
