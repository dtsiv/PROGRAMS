#include "qrmoconnection.h"
#include "qrmoconnection_p.h"

#include "connectionthreadworker.h"
#include "sendthreadworker.h"
#include "recvthreadworker.h"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QRmoConnection::QRmoConnection(
	QString sHost, int iPort, bool bDebugProto/*=false*/, 
	TcpConnectionRoles connectionRole/*=ClientRole*/, QObject *parent/*=0*/)
        : QObject(parent), d_ptr(new QRmoConnectionPrivate())
{
    Q_D(QRmoConnection);

    d->q_ptr = this;
    d->init(sHost,iPort,connectionRole,bDebugProto,parent);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QRmoConnection::QRmoConnection(
	int iPort, bool bDebugProto/*=false*/, 
	TcpConnectionRoles connectionRole/*=ServerRole*/, 
	QString sHost/*="0.0.0.0"*/, QObject *parent/*=0*/)
     : QObject(parent) , d_ptr(new QRmoConnectionPrivate())
{
    Q_D(QRmoConnection);

    d->q_ptr = this;
    d->init(sHost,iPort,connectionRole,bDebugProto,parent);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QRmoConnection::~QRmoConnection() {

    Q_D(QRmoConnection);

    // emit cleanup() signal for graceful shutdown.
    d->m_pConnectionThreadWorker->cleanup();

    // wait for ConnectionThread to finish
	if (d->m_pConnectionThread->isRunning()) {
        if (!d->m_pConnectionThread->wait(QRMO_CONNECTION_WAIT_TIMEOUT_MSEC)) {
            qDebug() << "m_pConnectionThread->isRunning()=" << d->m_pConnectionThread->isRunning();
            // throw QRmoConnectionException("~QRmoConnection() m_pConnectionThread->wait failed");
        }
    }
    // wait for SendThread to finish
	if (d->m_pSendThread->isRunning()) {
        if (!d->m_pSendThread->wait(QRMO_CONNECTION_WAIT_TIMEOUT_MSEC)) {
            qDebug() << "m_pSendThread->isRunning()=" << d->m_pSendThread->isRunning();
            // throw QRmoConnectionException("~QRmoConnection() m_pSendThread->wait failed");
        }
    }
    // wait for RecvThread to finish
	if (d->m_pRecvThread->isRunning()) {
        if (!d->m_pRecvThread->wait(QRMO_CONNECTION_WAIT_TIMEOUT_MSEC)) {
            qDebug() << "m_pRecvThread->isRunning()=" << d->m_pRecvThread->isRunning();
            // throw QRmoConnectionException("~QRmoConnection() m_pRecvThread->wait failed");
        }
    }
    // m_pConnectionThreadWorker, m_pSendThreadWorker, m_pRecvThreadWorker objectes will be deleted by deleteLater() slot
    // m_pConnectionThread, m_pSendThread, m_pRecvThread threads have finished with guarantee
    //delete d->m_pConnectionThreadWorker;
    delete d->m_pConnectionThread;
    //delete d->m_pSendThreadWorker;
    delete d->m_pSendThread;
    //delete d->m_pRecvThreadWorker;
    delete d->m_pRecvThread;
    delete d_ptr;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
bool QRmoConnection::isConnected() {

    Q_D(QRmoConnection);

	return d->m_pConnectionThreadWorker->isConnected();
}

//-----------------------------------------------------------------------
//   initiate graceful shutdown
//-----------------------------------------------------------------------
void QRmoConnection::doDisconnect() {

    Q_D(QRmoConnection);

    // forbid further transfer and release waiting send threads
    // User gets informed by disconnected() signal
    d->m_pConnectionThreadWorker->closeConnection();
}

//-----------------------------------------------------------------------
//   initiate graceful shutdown
//-----------------------------------------------------------------------
void QRmoConnection::reconnect(QString sHost, int iPort) {

    Q_D(QRmoConnection);

	if (iPort != qBound(1,iPort,65535) || sHost.isEmpty()) {
		qDebug() << "QRmoConnection::reconnect(). Invalid host/port: " << sHost << ":" << iPort;
        // throw QRmoConnectionException("QRmoConnection::reconnect(). Invalid host/port: ");
	}
	if (d->m_sHost != sHost || d->m_iPort != iPort) {
		d->m_sHost = sHost;
		d->m_iPort = iPort;
        d->m_pConnectionThreadWorker->reconnect();
	}
}

//-----------------------------------------------------------------------
// QRmoConnection::acceptPayload is the function used for sending payload
// over network. This function is thread-safe.
// The calling thread blocks until timeout (msec) elapses. 
// For timeout=ULONG_MAX the function does not timeout, see QWaitCondition::wait()
// The function returns false if (1) timeout occurs or (2) error occurs.
// The error description is returned in qsErrorString
//-----------------------------------------------------------------------
bool QRmoConnection::send(
	QByteArray *pbaPayload, 
	int iType, 
	unsigned long timeout/*=ULONG_MAX*/) {

    Q_D(QRmoConnection);

	return d->m_pSendThreadWorker->lockSend(pbaPayload,iType,timeout);

}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QAbstractSocket::SocketError QRmoConnection::error() {

    Q_D(QRmoConnection);

	return d->m_pSendThreadWorker->m_iErrorCode;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QString QRmoConnection::errorString() {

    Q_D(QRmoConnection);

	return d->m_pSendThreadWorker->m_qsErrorString;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
bool QRmoConnection::waitForDisconnected(unsigned long iTimeout /*=-1*/) {

    Q_D(QRmoConnection);

	// protect from parallel threads read-write pitfalls
	QMutexLocker locker(d->getWaitForDisconnectedMutex());

    // return true if current connection state is not QAbstractSocket::ConnectedState
	if (!d->m_pConnectionThreadWorker->isConnected()) return true;

	// otherwise wait for disconnected signal
	return d->m_waitForDisconnectedCondition.wait(d->getWaitForDisconnectedMutex(), iTimeout);
}
//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::wakeOnDisconnected() {
    // wake all threads waiting for waitForDisconnect() method
	m_waitForDisconnectedCondition.wakeAll();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::dispatchPayload(QByteArray *pbaPayload, int iType) {

    Q_Q(QRmoConnection);

    emit q->receive(pbaPayload, iType);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::connectedStateChanged(QString sHost, int iPort, bool bConnected) {

    Q_Q(QRmoConnection);

	if (!bConnected) { // disconnected
	    emit q->disconnected();
	}
	else { // connected
	    emit q->connected(sHost,iPort);
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::signalError(QAbstractSocket::SocketError iErrorCode,QString qsErrorString) {

    Q_Q(QRmoConnection);

    emit q->error(iErrorCode, qsErrorString);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
QMutex * QRmoConnectionPrivate::getWaitForDisconnectedMutex() {
    return &m_waitForDisconnectedMutex;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::init(
    QString sHost,int iPort,QRmoConnection::TcpConnectionRoles connectionRole, 
    bool bDebugProto,QObject *parent)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    m_sHost = sHost;
    m_iPort = iPort;
	m_role = connectionRole;
	m_bDebugProto = bDebugProto;

    // start connection thread
    m_pConnectionThread = new QThread;
    m_pConnectionThreadWorker = new ConnectionThreadWorker(this, m_pConnectionThread);
    m_pConnectionThreadWorker->moveToThread(m_pConnectionThread);

    // start thread to send data
    m_pSendThread = new QThread;
    m_pSendThreadWorker = new SendThreadWorker(this);
    m_pSendThreadWorker->moveToThread(m_pSendThread);

    // start thread to send data
    m_pRecvThread = new QThread;
    m_pRecvThreadWorker = new RecvThreadWorker(this);
    m_pRecvThreadWorker->moveToThread(m_pRecvThread);

	m_pRecvThreadWorker->m_pConnWorker = m_pConnectionThreadWorker;
	m_pSendThreadWorker->m_pConnWorker = m_pConnectionThreadWorker;
    
	m_pConnectionThreadWorker->m_pSendThreadWorker=m_pSendThreadWorker;
	m_pConnectionThreadWorker->m_pRecvThreadWorker=m_pRecvThreadWorker;

	// signalling between worker threads
	QObject::connect(
        m_pSendThreadWorker,
        SIGNAL(doSend(QByteArray*, int)),
        m_pConnectionThreadWorker,
        SLOT(onDoSend(QByteArray*, int)));
    QObject::connect(
        m_pConnectionThreadWorker,
        SIGNAL(sendOk()),
        m_pSendThreadWorker,
        SLOT(unlockSend()));
    QObject::connect(
        m_pConnectionThreadWorker,
        SIGNAL(doRecv(QByteArray*,int)),
        m_pRecvThreadWorker,
        SLOT(onDoRecv(QByteArray*,int)));
    QObject::connect(
        m_pRecvThreadWorker,
        SIGNAL(recvOk()),
        m_pConnectionThreadWorker,
        SLOT(onRecvOk()));

	// start all threads
	startThreads();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void QRmoConnectionPrivate::startThreads() {
    // start Connection Thread
    m_pConnectionThread->start();
    // start send
    m_pSendThread->start();
    // start recv
    m_pRecvThread->start();
}
