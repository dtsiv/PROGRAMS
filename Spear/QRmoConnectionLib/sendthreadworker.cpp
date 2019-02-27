#include "sendthreadworker.h"
#include "connectionthreadworker.h"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
SendThreadWorker::SendThreadWorker(QRmoConnectionPrivate * pRmoConnectionPrivate)
    : QObject(0)
	, m_iErrorCode(QAbstractSocket::UnknownSocketError)
	, m_qsErrorString(QString("QRmoConnection: Not connected."))
	, m_bError(true)
        , m_pRmoConnectionPrivate(pRmoConnectionPrivate)
{
    // error signal
	QObject::connect(
		this,
		SIGNAL(error(QAbstractSocket::SocketError,QString)),
        SLOT(onError(QAbstractSocket::SocketError,QString)));

    // clearError signal
	QObject::connect(
		this,
		SIGNAL(clearError()),
        SLOT(onClearError()));

    // cleanup
    QObject::connect(
        this,
        SIGNAL(cleanupNonBlocking()),
        SLOT(onCleanup()));
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
SendThreadWorker::~SendThreadWorker() {
	// on error, m_senders is not cleared - just waked all waiting threads
	while (!m_senders.isEmpty()) {
        delete m_senders.dequeue();
	}
    QObject::thread()->quit();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
bool SendThreadWorker::lockSend(
    QByteArray *pbaPayload, int iType, unsigned long timeout/* = ULONG_MAX*/) {
	// use QMutexLocker due to many return points
    QMutexLocker locker(&m_mutex);
	// no further operation on error
	if (m_bError) return false;
#ifdef ERROR_CONTROL_LEVEL_DEBUG
	// protect the algorithm from invalid external input
	if (!pbaPayload) {
		m_bError=true;
		m_iErrorCode=QAbstractSocket::UnknownSocketError;
		m_qsErrorString = "QRmoConnection: zero pointer to payload";
		return false;
	}
    // check if byte array is null
	if (pbaPayload->isNull()) {
		m_bError=true;
		m_iErrorCode=QAbstractSocket::UnknownSocketError;
		m_qsErrorString = "QRmoConnection: pbaPayload->isNull()";
		return false;
	}
#endif
	// Arrange a new SendRequestInfo for the current sender
	// This function ONLY is responsible for allocating and deleteing 
	// its SendRequestInfo structure called currentSender
	SendRequestInfo * currentSender = new SendRequestInfo();
    // See if there are already senders waiting in the queue
	if (!m_senders.isEmpty()) {
		// In QQueue, the most recent waiter is called "last()"
	    SendRequestInfo * lastSender = m_senders.last();
		// ANYWAY need to append the current Sender to m_senders queue. 
	    m_senders.enqueue(currentSender);
		// Wait for most recent sender to complete
		bool bWaitResult = lastSender->waitConditionNextEnabled.wait(&m_mutex,timeout);
		// On error - wake next sender, remove current sender from queue and delete it
        if ( m_bError ) { // ("terminus B")
			// wake the next waiting thread
			currentSender->waitConditionNextEnabled.wakeOne();
			// signal failure to end-user
			return false;
		}
		// On timeout: set error status, remove sender from queue and return false 
		if ( !bWaitResult ) { // ("terminus C")
			// Timeout is a fatal error for the whole queue. (Reconnect required)
			m_bError=true;
			m_iErrorCode=QAbstractSocket::UnknownSocketError;
			m_qsErrorString = "QRmoConnection: timeout.";
			// wake the next waiting thread
			currentSender->waitConditionNextEnabled.wakeOne();
			// signal failure to end-user
			return false;
		}
		// There no error. Its our turn to send payload. 
		// Here we must signal waitConditionSendComplete for the lastSender 
		lastSender->waitConditionSendComplete.wakeOne();
	}
	else { // Queue is empty. The currentSender will be at the head of the queue
	    m_senders.enqueue(currentSender);
	}
	//------------------------------ main route ("terminus A") ------------------------------
	emit doSend(pbaPayload, iType);
	// wait for send to complete
	bool bWaitResult = currentSender->waitConditionSendComplete.wait(&m_mutex,timeout);
	// no further operation on error
    if ( m_bError ) {
		// wake the next waiting thread
		currentSender->waitConditionNextEnabled.wakeOne();
		// signal failure to end-user
		return false;
	}
	// On timeout: set error status, remove sender from queue and return false 
	if ( !bWaitResult ) {
		// Timeout is a fatal error for the whole queue. (Reconnect required)
		m_bError=true;
		m_iErrorCode=QAbstractSocket::UnknownSocketError;
		m_qsErrorString = "QRmoConnection: timeout.";
		// wake the next waiting thread
		currentSender->waitConditionNextEnabled.wakeOne();
		// signal failure to end-user
		return false;
	}
	// delete currentSender. Next sender is not waiting any more
	delete currentSender;
	// return current error status. m_iErrorCode and m_qsErrorString show details
	return (!m_bError);
	// QByteArray *pbaPayload is not needed any more
	// The calling thread is responsible for calling "delete pbaPayload;"
}

//-----------------------------------------------------------------------
//    this slot is connected to sendOk() signal (see the onBytesWritten() slot)
//-----------------------------------------------------------------------
void SendThreadWorker::unlockSend() {
    QMutexLocker locker(&m_mutex);
#ifdef ERROR_CONTROL_LEVEL_DEBUG
	// there must be at least one sender waiting (the one who emitted doSend())
	if (m_senders.isEmpty()) {
        m_bError=true;
        m_iErrorCode=QAbstractSocket::UnknownSocketError;
		m_qsErrorString="QRmoConnection: Bad queue state (2).";
		return;
	}
#endif
	// wake the OLDEST waiter. 
    // In QQueue the OLDEST waiter is called "first" or head()
	// enqueue() ---> (__last__) (OOO) (OOO) ... (OOO) (__first__) ---> dequeue()
	SendRequestInfo * pSender = m_senders.dequeue();
	// if there is no next sender - wake up main route "terminus A"
	if(m_senders.isEmpty()) {
		pSender->waitConditionSendComplete.wakeOne();
	}
	// if there is next sender waiting for waitConditionNextEnabled
	else {
		pSender->waitConditionNextEnabled.wakeOne();
	}
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void SendThreadWorker::afterConnected() {
    emit clearError();
}

//-----------------------------------------------------------------------
//  called after successful connect. The queue must be restarted
//-----------------------------------------------------------------------
void SendThreadWorker::onClearError() {
	// lock send mutex
    QMutexLocker locker(&m_mutex);
	// there may be non-empty m_senders queue after last error. Clean it up
	while (!m_senders.isEmpty()) {
		delete m_senders.dequeue();
	}
	// clear current error flag
	m_bError=false;
	// set current error information
	m_iErrorCode = QAbstractSocket::UnknownSocketError;
	m_qsErrorString = QString();
    // clear stop flag of connectionthread, set connected status and emit connected
	// this slot can be called directly
    m_pConnWorker->startTransfer();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void SendThreadWorker::raiseError(QAbstractSocket::SocketError iErrorCode,QString qsErrorString) {
	// emit
    emit error(iErrorCode,qsErrorString);
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void SendThreadWorker::onError(QAbstractSocket::SocketError iErrorCode,QString qsErrorString) {
	// lock send mutex
    QMutexLocker locker(&m_mutex);
	// set current error flag
	m_bError=true;
	// set current error information
	m_iErrorCode = iErrorCode;
	m_qsErrorString = qsErrorString;
	// wake all waiting senders to let them return
	if (!m_senders.isEmpty()) {
		QQueue<SendRequestInfo*>::iterator i;
		for (i = m_senders.begin(); i != m_senders.end(); ++i) {
			(*i) -> waitConditionNextEnabled.wakeOne();
			(*i) -> waitConditionSendComplete.wakeOne();
		}
	}
	// signal error to user
	m_pRmoConnectionPrivate->signalError(iErrorCode, qsErrorString);
}


//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void SendThreadWorker::cleanup() {
    // emit
    emit cleanupNonBlocking();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void SendThreadWorker::onCleanup() {
    deleteLater();
}
