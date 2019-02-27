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
    , m_bIsNextBlockHeader(true)
    , m_iTotalBytesRead(0)
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
    // non-blocking signal cleanup()
    QObject::connect(
        this,
        SIGNAL(cleanupNonBlocking()),
        SLOT(onCleanup()));
    // non-blocking version for API function doDisconnect()
    QObject::connect(
        this,
        SIGNAL(doDisconnect()),
        SLOT(onDoDisconnect()));
	// doReconnect() signal
    QObject::connect(
		this,
        SIGNAL(doReconnect()),
        SLOT(onDoReconnect()));
    // readyForTransfer() signal
    QObject::connect(
        this,
        SIGNAL(readyForTransfer()),
        SLOT(onReadyForTransfer()));
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
ConnectionThreadWorker::~ConnectionThreadWorker() {
    // IMPORTANT!!! ~ConnectionThreadWorker() is NEVER!!! called from another thread
    if (m_pTimer) {
        delete m_pTimer;
    }
    // all bytes are written by now
    if (m_pTcpSocket) m_pTcpSocket->close();
    // In case of QRmoConnection::ServerRole - m_pTcpSocket has m_pTcpServer as QObject parent
	if (m_pTcpSocket) delete m_pTcpSocket;
	// delete m_pTcpServer as last. OTHERWISE DELETING CHILD AFTER PARENT leads to crash!
    if (m_pTcpServer) delete m_pTcpServer;
    QObject::thread()->quit();
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
    emit doDisconnect();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::cleanup() {
	// stop the timer, close server, socket. Emit blocks until slot finishes
	// The emit blocks until slot onCleanup() finishes (Qt::BlockingQueuedConnection)
    emit cleanupNonBlocking();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onDoDisconnect() {
    // deactivate connection attempts, send transfer
    m_bStop = true;
    // release send queue
    m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"Disconnected at user request");
    // stop timer of connection attempts
    if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ClientRole) {
        // stop timer and cancel connection attempts
        if (m_pTimer) m_pTimer->stop();
    }
    // close server
    if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ServerRole) {
        // close() means stop listening
        m_pTcpServer->close();
    }
    // graceful shutdown of tcpsocket
    if (m_pTcpSocket) {
        if (m_pTcpSocket->state() != QAbstractSocket::UnconnectedState) {
            // disconnectFromHost() will wait for pending bytes
            m_pTcpSocket->disconnectFromHost();
        }
    }
    // TRICK!! avoid long waiting to close socket
    m_bConnected = false;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onCleanup() {
    // !!! IMPORTANT !!! to be called ONLY from m_pConnectionThread thread
	// stopping flag for worker threads
	m_bStop=true;
    m_iTotalBytesRead=0;
    m_bIsNextBlockHeader=true;
    m_baPartialBuffer.clear();
	// stop timer of connection attempts
    if (m_pRmoConnectionPrivate->m_role == QRmoConnection::ClientRole) {
		// stop timer and cancel connection attempts
		if (m_pTimer) m_pTimer->stop();
	}
	// release waiting send threads
	m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: cleanup in progress");
    m_pSendThreadWorker->cleanup();
    // stop recv thread
    m_pRecvThreadWorker->cleanup();
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
		}
	}
    deleteLater();
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
        // throw QRmoConnectionException("m_pTcpServer->listen() failed");
        qDebug() << "m_pTcpServer->listen() failed";
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
    // !!! IMPORTANT !!! to be called ONLY from m_pConnectionThread thread
    // stopping flag for worker threads
    m_iTotalBytesRead=0;
    m_bIsNextBlockHeader=true;
    m_baPartialBuffer.clear();
    // SendThreadWorker::afterConnected() cleanup send queue and enable send
	m_pSendThreadWorker->afterConnected();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::startTransfer() {
    emit readyForTransfer();
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
    // !!! IMPORTANT !!! to be called ONLY from m_pConnectionThread thread
    // stopping flag for worker threads
    m_bStop=true;
    m_iTotalBytesRead=0;
    m_bIsNextBlockHeader=true;
    m_baPartialBuffer.clear();
    // block send transfer
    m_pSendThreadWorker->raiseError(QAbstractSocket::UnknownSocketError,"QRmoConnection: Not connected.");
    // inform all users about connection loss
    m_pRmoConnectionPrivate->connectedStateChanged(QString(),0,false);
    // protect from parallel threads read-write pitfalls
    QMutexLocker locker(m_pRmoConnectionPrivate->getWaitForDisconnectedMutex());
	// not it is safe to clear m_bConnected flag
    m_bConnected = false;
	// release threads waiting for disconnected
	m_pRmoConnectionPrivate->wakeOnDisconnected();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onRecvOk() {
	// no further operation if stop flag raised
	if (m_bStop) return;
    // read newly arrived data (if any)
	readPayload();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::onReadyRead() {
	// no further operation if stop flag raised
	if (m_bStop) return;
	// otherwise proceed with ordinary receive
	readPayload();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void ConnectionThreadWorker::readPayload() {
    qint64 iProtoHeaderSize, bytesRead, bytesAvailable;
    qint64 iBytesToRead;

    // prepare for header receive
    iProtoHeaderSize = sizeof(m_receiveHeader.cdata);
    if(!m_pRmoConnectionPrivate->m_bDebugProto) iProtoHeaderSize -= sizeof(unsigned int);
    // read headers or header+buffer, if available
    if (m_bIsNextBlockHeader) {
        // read m_receiveHeader structures from socket until nonzero payload
        do {
            bytesAvailable = m_pTcpSocket->bytesAvailable();
            if (!bytesAvailable) return;
            if (!m_iTotalBytesRead) {
                m_baPartialBuffer.resize(iProtoHeaderSize);
            }
            if (m_iTotalBytesRead>=iProtoHeaderSize) {
                throw QRmoConnectionException(QString("m_receiveHeader: m_iTotalBytesRead(%1)>=iProtoHeaderSize(%2); bytesAvailable=%3")
                                              .arg(m_iTotalBytesRead).arg(iProtoHeaderSize).arg(bytesAvailable));
            }
            // if not enough data then use baPartialBuffer
            iBytesToRead=qMin(bytesAvailable,iProtoHeaderSize-m_iTotalBytesRead);
            if (m_iTotalBytesRead+iBytesToRead != iProtoHeaderSize) {
                throw QRmoConnectionException(QString("m_iTotalBytesRead(%1)+iBytesToRead(%2) != iProtoHeaderSize(%3)")
                                              .arg(m_iTotalBytesRead).arg(iBytesToRead).arg(iProtoHeaderSize));
            }
            bytesRead = m_pTcpSocket->read(m_baPartialBuffer.data()+m_iTotalBytesRead, iBytesToRead);
            if (bytesRead != iBytesToRead) throw QRmoConnectionException("bytesRead != iBytesToRead");
            m_iTotalBytesRead+=bytesRead;
            if (m_iTotalBytesRead < iProtoHeaderSize) {
                throw QRmoConnectionException(QString("return: m_iTotalBytesRead(%1) < iProtoHeaderSize(%2); bytesAvailable=%3")
                                              .arg(m_iTotalBytesRead).arg(iProtoHeaderSize).arg(bytesAvailable));
                 // complete header is not available at this time
                return;
            }
#ifdef ERROR_CONTROL_LEVEL_DEBUG
            else if (m_iTotalBytesRead != iProtoHeaderSize) {
                throw QRmoConnectionException(QString("m_iTotalBytesRead (%1) != iProtoHeaderSize (%2)").arg(m_iTotalBytesRead).arg(iProtoHeaderSize));
            }
#endif
            // must be m_iTotalBytesRead == iProtoHeaderSize
            else {
                  // the data is enough then read directly to m_receiveHeader
                memcpy(&m_receiveHeader.cdata[0],m_baPartialBuffer.data(),iProtoHeaderSize);
                  // header.iSize <= 0 is not an error for ADU
                  // if (m_receiveHeader.iSize <= 0) throw QRmoConnectionException("header.iSize <= 0");
                // no need to clear it here
                // baPartialBuffer.clear();
                m_nNextBlockSize = m_receiveHeader.iSize;
                //if (m_nNextBlockSize!=4096) {
                //    throw QRmoConnectionException(QString("m_receiveHeader.iSize = %1; m_receiveHeader.iType = %2; m_iTotalBytesRead=%3; iProtoHeaderSize=%4; bytesRead=%5; iBytesToRead=%6; m_nNextBlockSize=%7; ")
                //                  .arg(m_receiveHeader.iSize).arg(m_receiveHeader.iType).arg(m_iTotalBytesRead).arg(iProtoHeaderSize).arg(bytesRead).arg(iBytesToRead).arg(m_nNextBlockSize));
                //}
                m_iTotalBytesRead=0;
            }
            if (m_iTotalBytesRead>= iProtoHeaderSize) {
                throw QRmoConnectionException(QString("m_iTotalBytesRead(%1); iBytesToRead(%2); iProtoHeaderSize(%3)")
                                              .arg(m_iTotalBytesRead).arg(iBytesToRead).arg(iProtoHeaderSize));
            }
            if (m_nNextBlockSize == 0) throw QRmoConnectionException("do: m_nNextBlockSize == 0");
        } while (m_nNextBlockSize == 0);
        m_bIsNextBlockHeader=false;
        m_baPartialBuffer.resize(m_nNextBlockSize);
    }
    if (m_bIsNextBlockHeader == true || m_nNextBlockSize == 0 || m_nNextBlockSize>m_baPartialBuffer.size()) {
        throw QRmoConnectionException(QString("bIsNextBlockHeader(%1) == true || m_nNextBlockSize(%2) == 0 || m_nNextBlockSize (%3) > baPartialBuffer.size() (%4)")
                                      .arg(m_bIsNextBlockHeader).arg(m_nNextBlockSize).arg(m_nNextBlockSize).arg(m_baPartialBuffer.size()));
    }
	// Ok, now expecting m_nNextBlockSize = header.iSize
    bytesAvailable = m_pTcpSocket->bytesAvailable();
    if (!bytesAvailable) return;
    QByteArray payload;
#pragma GCC diagnostic ignored "-Wsign-compare"
    if (m_iTotalBytesRead || bytesAvailable < m_nNextBlockSize) {
        if (m_iTotalBytesRead >= m_nNextBlockSize) {
            throw QRmoConnectionException(QString("Payload: m_iTotalBytesRead(%1)>= m_nNextBlockSize(%2);")
                                          .arg(m_iTotalBytesRead).arg(m_nNextBlockSize));
            return;
        }

        // if not enougn data - then use baPartialBuffer
        iBytesToRead=qMin((qint64)bytesAvailable,(qint64)(m_nNextBlockSize-m_iTotalBytesRead));
        if (iBytesToRead <=0) throw QRmoConnectionException("iBytesToRead <=0");
        bytesRead = m_pTcpSocket->read(m_baPartialBuffer.data()+m_iTotalBytesRead, iBytesToRead);
        if (bytesRead != iBytesToRead) throw QRmoConnectionException("bytesRead != iBytesToRead");
        m_iTotalBytesRead+=bytesRead;
        if (m_iTotalBytesRead < m_nNextBlockSize) {
             // complete header is not available at this time
            return;
        }
        payload = QByteArray(m_baPartialBuffer);
        // baPartialBuffer is implicitly shared, try to avoid copy-on-write
        // baPartialBuffer.clear();
    }
    else {
        // It is now possible to read whole payload.
        // read payload from socket
        payload = m_pTcpSocket->read(m_nNextBlockSize);
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
    }
    // additional size check
    if (payload.isEmpty() || payload.size() != m_nNextBlockSize) {
        throw QRmoConnectionException("payload.isEmpty() || payload.size() != m_nNextBlockSize");
    }
    // dispatch the received payload
    emit doRecv(new QByteArray(payload),m_receiveHeader.iType);
    // reset to initial state
    m_bIsNextBlockHeader=true;
    // again start reading from 0 bytes
    m_iTotalBytesRead=0;
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
        // send protocol header. The amount bytesSent actually sent will be checked in onBytesWritten() slot
        int iProtoHeaderSize = sizeof(m_sendHeader.cdata);
        if(!m_pRmoConnectionPrivate->m_bDebugProto) iProtoHeaderSize -= sizeof(unsigned int);
        qint64 bytesSent;
        bytesSent = m_pTcpSocket->write((char *)&m_sendHeader.cdata[0], iProtoHeaderSize);
        if (bytesSent == -1) {
            m_bStop = true;
		    m_pSendThreadWorker->raiseError(m_pTcpSocket->error(),m_pTcpSocket->errorString());
			return;
        }
        // send payload. The amount bytesSent actually sent will be checked in onBytesWritten() slot
        bytesSent = m_pTcpSocket->write(payload);
        if (bytesSent == -1) {
            m_bStop = true;
		    m_pSendThreadWorker->raiseError(m_pTcpSocket->error(),m_pTcpSocket->errorString());
			return;
        }
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
    // !!! IMPORTANT !!! to be called ONLY from m_pConnectionThread thread
    // stopping flag for worker threads
    m_iTotalBytesRead=0;
    m_bIsNextBlockHeader=true;
    m_baPartialBuffer.clear();

	// release all senders 
	m_pSendThreadWorker->raiseError(iErrorCode,m_pTcpSocket->errorString());
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
