#include "connectionthreadworker.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
ConnectionThreadWorker::ConnectionThreadWorker(QRmoConnectionPrivate * pRmoConnectionPrivate, QThread * pConnectionThread)
    : QObject(0)
    , m_pRmoConnectionPrivate(pRmoConnectionPrivate)
	, m_pSendThreadWorker(0)
	, m_pRecvThreadWorker(0)
    , m_nNextBlockSize(0)
	, m_pTcpSocket(0)
	, m_pTcpServer(0)
    , m_pTimer(0)
    , m_bStop(false)
	, m_bConnected(false)
	, m_bRecvUnlocked(true) 
{
	// if we are server - then initialize the member pointer
	if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ServerRole) {
		// create TCP server with no parent (in order to change thread later)
		m_pTcpServer = new QTcpServer(0);
		// m_TcpServer is initially in GUI thread. Need to change affinity by hand
		m_pTcpServer->moveToThread(pConnectionThread);
		// currently, only one client supported
		m_pTcpServer->setMaxPendingConnections(1);
		// connect the server to slots
		QObject::connect(
			m_pTcpServer,
			SIGNAL(newConnection()),
			SLOT(onNewConnection()));
	}
    // connect start signal of the main thread "ConnectionThread" to slot onStarted()
	// the slot onStarted() acts as the "entry point" of RmoConnection operations
    QObject::connect(
        pConnectionThread,
        SIGNAL(started()),
        SLOT(onStarted()));
    // the cleanup() signal is emitted from GUI thread 
	// by delete QRmoConnection instance. 
	// Slot connection type needed: Qt::BlockingQueuedConnection
    QObject::connect(
        this,
        SIGNAL(cleanupBlocking()),
        SLOT(onCleanup()),
        Qt::BlockingQueuedConnection);
	// same signal, non-blocking version for API function doDisconnect()
    QObject::connect(
        this,
        SIGNAL(cleanupNonBlocking()),
        SLOT(onCleanup()));
	// doReconnect() signal
    QObject::connect(
		this,
        SIGNAL(doReconnect()),
        SLOT(onDoReconnect()));
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
ConnectionThreadWorker::~ConnectionThreadWorker() {
	// ~ConnectionThreadWorker() is called from another thread, but the member objects are not more active
	// and the object's thread is finished() by the time of destructor call
    if (m_pTimer) {
        delete m_pTimer;
    }
	// In case of QRmoConnection::ServerRole - m_pTcpSocket has m_pTcpServer as QObject parent
	if (m_pTcpSocket) delete m_pTcpSocket;
	// delete m_pTcpServer as last. OTHERWISE DELETING CHILD AFTER PARENT leads to crash!
    if (m_pTcpServer) delete m_pTcpServer;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onStarted() {
	// this is starting point of the ConnectionThread. No connection exists so far. Signal it!
	m_pRmoConnectionPrivate->connectedStateChanged(QString(),0,false);
    // start as client
	if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ClientRole) {
		// create members in object's thread
		m_pTcpSocket=new QTcpSocket;
		setSocketOptions(m_pTcpSocket);
		connectSlotsToSocket(m_pTcpSocket);
		// timer will have pConnectionThread affinity
		m_pTimer = new QTimer(this);
		// connect thread to slots
		QObject::connect(
			m_pTimer,
			SIGNAL(timeout()),
			SLOT(tryConnect()));
		// start timer with timeout interval (msec), make connection attempt
		m_pTimer->start(QRMO_CONNECTION_POLLING_TIMEOUT_MSEC);
		tryConnect();
	}
    // start as server
	if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ServerRole) {
		if (!m_pTcpServer)
            throw QRmoConnectionException("fatal (m_pTcpServer=0)");
		startListen();
	}
}

//-----------------------------------------------------------------------
//  this slot is enabled for server only
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onNewConnection() {
	// protext from unexpected error
	if (!m_pTcpServer->hasPendingConnections()) 
        throw QRmoConnectionException("ConnectionThreadWorker::onNewConnection(). No pending connections.");
	// if the member socket is alive and connected - skip the incoming connection 
	if (m_pTcpSocket) {
		// Currently, only one connection is supported for server
		if (m_pTcpSocket->state()==QAbstractSocket::ConnectedState) {
            if (m_pTcpServer->hasPendingConnections()) {
	            QTcpSocket * pSocket = m_pTcpServer->nextPendingConnection();
				pSocket->disconnectFromHost();
				pSocket->deleteLater();
			}
			return;
		}
        // socket not connected: delete the socket
		else {
            m_pTcpSocket->disconnectFromHost();
            m_pTcpSocket->close();
			m_pTcpSocket->deleteLater();
			m_pTcpSocket=0;
		}
	}
	// now server is ready to accept new incoming connection
	QTcpSocket * pSocket = m_pTcpServer->nextPendingConnection();
	// assign the member socket pointer and initialize it
	m_pTcpSocket = pSocket;
	setSocketOptions(m_pTcpSocket);
	connectSlotsToSocket(m_pTcpSocket);
	// enable receive
	m_bRecvUnlocked = true;
	// SendThreadWorker::afterConnected() cleanup send queue, enable send, emit connected
	m_pSendThreadWorker->afterConnected();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
bool ConnectionThreadWorker::isConnected() {
	// m_bConnected is set in onReadyForTransfer()
	return m_bConnected;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::closeConnection() {
	// block further receive operation 
	m_bRecvUnlocked=false;
	// release send queue
	m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"Disconnected at user request");
	// deactivate connection attempts, send transfer
	m_bStop = true;
	// stop the timer, close server, socket. 
	// The emit calls slot onCleanup() using Qt::AutoConnection
    emit cleanupNonBlocking();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::cleanup() {
	// stop the timer, close server, socket. Emit blocks until slot finishes
	// The emit blocks until slot onCleanup() finishes (Qt::BlockingQueuedConnection)
    emit cleanupBlocking();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onCleanup() {
	// stopping flag for worker threads
	m_bStop=true;
	// stop timer of connection attempts
    if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ClientRole) {
		// stop timer and cancel connection attempts
		if (m_pTimer) m_pTimer->stop();
	}
	// release waiting send threads
	m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: cleanup in progress");
	// stop receive operation
	m_bRecvUnlocked=false;
	// close server
	if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ServerRole) {
		// close() means stop listening
		m_pTcpServer->close();
	}
    // graceful shutdown of tcpsocket
	if (m_pTcpSocket) {
		if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState) {
			// disconnectFromHost() will wait for pending bytes
			m_pTcpSocket->disconnectFromHost();
			if (m_pTcpSocket->state() != QAbstractSocket::UnconnectedState) {
				if (!m_pTcpSocket->waitForDisconnected(30000)) 
					throw QRmoConnectionException(QString("ConnectionThreadWorker::onCleanup() failed to disconnect. ErrorString=%1")
						.arg(m_pTcpSocket->errorString()));
			}
		}
		// all bytes are written by now
		m_pTcpSocket->close();
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::setSocketOptions(QTcpSocket *pSocket) {
	// --- MFC library reference (afxsock.h): ---
	// The TCP_NODELAY option disables the Nagle algorithm.
	// The Nagle algorithm is used to reduce the number
	// of small packets sent by a host by buffering unacknowledged send data
	// until a full-size packet can be sent.
	// However, for some applications this algorithm can impede performance,
	// and TCP_NODELAY can be used to turn it off.
	// Application writers should not set TCP_NODELAY unless the impact
	// of doing so is well-understood and desired,
	// since setting TCP_NODELAY can have a significant negative impact on network performance.
	// TCP_NODELAY is the only supported socket option which uses level IPPROTO_TCP;
	// all other options use level SOL_SOCKET.
	// --- enum QAbstractSocket::SocketOption ---
	// QAbstractSocket::LowDelayOption   Value=0
	// Description Try to optimize the socket for low latency. For a QTcpSocket
	// this would set the TCP_NODELAY option and disable Nagle's algorithm. Set this to 1 to enable.
	pSocket->setSocketOption(QAbstractSocket::LowDelayOption,QRMO_CONNECTION_OPTION_LOW_DELAY);
	// --- MFC library reference (afxsock.h): ---
	// An application can request that the Windows Sockets implementation
	// enable the use of "keep-alive" packets on Transmission Control Protocol (TCP) connections
	// by turning on the SO_KEEPALIVE socket option.
	// A Windows Sockets implementation need not support the use of keep-alives: if it does,
	// the precise semantics are implementation-specific but should conform
	// to section 4.2.3.6 of RFC 1122: "Requirements for Internet Hosts — Communication Layers."
	// If a connection is dropped as the result of "keep-alives" the error code WSAENETRESET
	// is returned to any calls in progress on the socket, and any subsequent calls will fail with WSAENOTCONN
	// --- enum QAbstractSocket::SocketOption ---
	// QAbstractSocket::KeepAliveOption Value=1
	// Description Set this to 1 to enable the SO_KEEPALIVE socket option
	pSocket->setSocketOption(QAbstractSocket::KeepAliveOption,QRMO_CONNECTION_OPTION_KEEP_ALIVE);
	// cannot accept buffer larger than QRMO_CONNECTION_READ_BUFFER_SIZE!!
	pSocket->setReadBufferSize(QRMO_CONNECTION_READ_BUFFER_SIZE);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::connectSlotsToSocket(QTcpSocket *pSocket) {
	// connect the socket to slots
	QObject::connect(
		pSocket,
		SIGNAL(readyRead()),
		SLOT(onReadyRead()));
	QObject::connect(
		pSocket,
		SIGNAL(bytesWritten(qint64)),
		SLOT(onBytesWritten(qint64)));
	QObject::connect(
		pSocket,
		SIGNAL(disconnected()),
		SLOT(onDisconnected()));
	QObject::connect(
		pSocket,
		SIGNAL(connected()),
		SLOT(onConnected()));
	QObject::connect(
		pSocket,
		SIGNAL(error(QAbstractSocket::SocketError)),
		SLOT(onError(QAbstractSocket::SocketError)));
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::reconnect() {
	emit doReconnect();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onDoReconnect() {
    if (m_pTcpSocket && m_pTcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_pTcpSocket->disconnectFromHost();
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::startListen() {
    if (!m_pTcpServer->listen(
          QHostAddress(m_pRmoConnectionPrivate->m_sHost),
		  m_pRmoConnectionPrivate->m_iPort)) {
        throw QRmoConnectionException("m_pTcpServer->listen() failed");
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::tryConnect() {
	// no further operation if stop flag raised
    if (m_bStop) return;
    // Continue from QAbstractSocket::UnconnectedState only
    if (m_pTcpSocket->state() == QAbstractSocket::UnconnectedState) {
		// Tcp client makes connection attempt
        m_pTcpSocket->connectToHost(m_pRmoConnectionPrivate->m_sHost, m_pRmoConnectionPrivate->m_iPort);
    }
}

//-----------------------------------------------------------------------
//    the connected() signal in pactice is available for client only
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onConnected() {
	// enable receive
	m_bRecvUnlocked = true;
	// SendThreadWorker::afterConnected() cleanup send queue and enable send
	m_pSendThreadWorker->afterConnected();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onReadyForTransfer() {
    // clear stop flag
	m_bStop = false;
	// set QRmoConnection connected status
	m_bConnected = true;
	// inform the clients about established connection
	m_pRmoConnectionPrivate->connectedStateChanged(
		m_pRmoConnectionPrivate->m_sHost,
		m_pRmoConnectionPrivate->m_iPort,true);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onDisconnected() {
	// protect from parallel threads read-write pitfalls
	QMutexLocker locker(m_pRmoConnectionPrivate->getWaitForDisconnectedMutex());
	// not it is safe to clear m_bConnected flag
    m_bConnected = false;
	// inform all users about connection loss
    m_pRmoConnectionPrivate->connectedStateChanged(QString(),0,false);
	// block send transfer
	m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: Not connected.");
	// release threads waiting for disconnected
	m_pRmoConnectionPrivate->wakeOnDisconnected();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onRecvOk() {
	// no further operation if stop flag raised
	if (m_bStop) return;
	// unlock next receive
    m_bRecvUnlocked = true;
    // read newly arrived data (if any)
	readPayload();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onReadyRead() {
	// no further operation if stop flag raised
	if (m_bStop) return;
	// if receive is locked - do processing later within onRecvOk() slot
	if (!m_bRecvUnlocked) return;
	// otherwise proceed with ordinary receive
	readPayload();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::readPayload() {
    qint64 iProtoHeaderSize, bytesRead, bytesAvailable;
	// prepare for header receive
    iProtoHeaderSize = sizeof(m_receiveHeader.cdata);
    if(!m_pRmoConnectionPrivate->m_bDebugProto) iProtoHeaderSize -= sizeof(unsigned int);
	// read at most ONE header+buffer, if available
    if (!m_nNextBlockSize) {
        bytesAvailable = m_pTcpSocket->bytesAvailable();
		// if not enough data then skip
        if (bytesAvailable < iProtoHeaderSize) return;
        bytesRead = m_pTcpSocket->read((char *)&m_receiveHeader.cdata[0], iProtoHeaderSize);
		// check the bytes amount is right
#ifdef ERROR_CONTROL_LEVEL_DEBUG
        if (bytesRead != iProtoHeaderSize) throw QRmoConnectionException("bytesRead != iProtoHeaderSize");
		// header.iSize <= 0 is not an error for ADU
        // if (m_receiveHeader.iSize <= 0) throw QRmoConnectionException("header.iSize <= 0");
#endif
        m_nNextBlockSize = m_receiveHeader.iSize;
		// check for zero size (this is not an error for ADU)
        if (m_receiveHeader.iSize == 0) // next to come is again a header
			return;
    }
	// Ok, now expecting m_nNextBlockSize = header.iSize
    bytesAvailable = m_pTcpSocket->bytesAvailable();
	// if not enougn data - then skip
#pragma GCC diagnostic ignored "-Wsign-compare"
	if (bytesAvailable < m_nNextBlockSize) return;
	// It is now possible to read whole payload. But first lock recv until recvOk()
	m_bRecvUnlocked = false;
    // read payload from socket
    QByteArray payload = m_pTcpSocket->read(m_nNextBlockSize);
#ifdef ERROR_CONTROL_LEVEL_DEBUG
	// these error checks are in fact redundant
    if (payload.isEmpty()) 
        throw QRmoConnectionException(QString("payload.isEmpty()."
		    " Error: %1. bytesAvailable: %2. m_nNextBlockSize: %3")
			.arg(m_pTcpSocket->errorString())
			.arg(bytesAvailable)
			.arg(m_nNextBlockSize));
    if (payload.size() != m_nNextBlockSize) 
        throw QRmoConnectionException("payload.size() != m_nNextBlockSize");
#endif
	// checksum
	if (m_pRmoConnectionPrivate->m_bDebugProto) {
		unsigned int dwCS = 0, i = 0;
		while (i < m_receiveHeader.iSize) dwCS += payload.at(i++);
        // raise error on checksum mismatch
		if (m_receiveHeader.dwCS != dwCS)
			throw QRmoConnectionException(QString("checksum/type mismatch."
				" CS@header: %1, CS@buffer: %2, TY@header: %3, *(int*)buf: %4")
				.arg(m_receiveHeader.dwCS)
				.arg(dwCS)
				.arg(m_receiveHeader.iType)
				.arg(*((int*)payload.data())));
	}
    // dispatch the received payload
    emit doRecv(new QByteArray(payload),m_receiveHeader.iType);
    // reset to initial state
    m_nNextBlockSize = 0;
#pragma GCC diagnostic pop
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onDoSend(QByteArray *pbaPayload, int iType) {
    // no further operation on error
	if (m_bStop) return;
    // dereference the QByteArray * (no deep copy)
    QByteArray payload=*pbaPayload;
	// if socket state is ConnectedState then proceed
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_sendHeader.iSize = payload.size();
        m_sendHeader.iType = iType;
        // calculate checksum if needed
        if (m_pRmoConnectionPrivate->m_bDebugProto) {
            m_sendHeader.dwCS = 0;
            int i = 0;
            while (i < m_sendHeader.iSize) m_sendHeader.dwCS += payload.at(i++);
        }
        // send protocol header
        int iProtoHeaderSize = sizeof(m_sendHeader.cdata);
        if(!m_pRmoConnectionPrivate->m_bDebugProto) iProtoHeaderSize -= sizeof(unsigned int);
        qint64 bytesSent;
        bytesSent = m_pTcpSocket->write((char *)&m_sendHeader.cdata[0], iProtoHeaderSize);
        if (bytesSent == -1) {
            m_bStop = true;
		    m_pSendThreadWorker->raiseError(m_pTcpSocket->error(),m_pTcpSocket->errorString());
			return;
        }
        // send payload
        bytesSent = m_pTcpSocket->write(payload);
        if (bytesSent == -1) {
            m_bStop = true;
		    m_pSendThreadWorker->raiseError(m_pTcpSocket->error(),m_pTcpSocket->errorString());
			return;
        }
#ifdef ERROR_CONTROL_LEVEL_DEBUG
		if (bytesSent != payload.size())
            throw QRmoConnectionException(QString("ConnectThread::onDoSend error: bytesSent != payload.size()."
				" errorString: %1").arg(m_pTcpSocket->errorString()));
#endif
    }
	else { // otherwise release all senders to avoid pitfall
		m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: Not connected.");
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onBytesWritten(qint64 bytes) {
    // check for QAbstractSocket::ConnectedState. If yes, then wake the oldest sender
    if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState) {
		// only if no more data left to write can we signal sendOk(). Its paranoya, but still...
		if (m_pTcpSocket->bytesToWrite()==0) {
            // release user's send() call
			emit sendOk();
		}
		else {
            m_bStop = true;
			// return false from all user's send() calls
			m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: bytesToWrite not null.");
		}
	}
	else { // The socket is not connected
        m_bStop = true;
		// return false from all user's send() calls
		m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: Not connected.");
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onError(QAbstractSocket::SocketError iErrorCode) {
	// release all senders 
	m_pSendThreadWorker->raiseError(iErrorCode,m_pTcpSocket->errorString());
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
