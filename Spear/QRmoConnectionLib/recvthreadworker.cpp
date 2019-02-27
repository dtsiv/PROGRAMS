#include "recvthreadworker.h"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
RecvThreadWorker::RecvThreadWorker(QRmoConnectionPrivate * pRmoConnectionPrivate)
        : QObject(0)
        , m_pRmoConnectionPrivate(pRmoConnectionPrivate) {

    // cleanup
    QObject::connect(
        this,
        SIGNAL(cleanupNonBlocking()),
        SLOT(onCleanup()));
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
RecvThreadWorker::~RecvThreadWorker() {
    QObject::thread()->quit();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void RecvThreadWorker::onDoRecv(QByteArray *pbaPayload, int iType) {
	// dispatchPayload() emits signal intended for
	// QRmoConnection::UniqueBlockingQueuedConnection 
	// which is defined as (Qt::ConnectionType)(Qt::BlockingQueuedConnection|Qt::UniqueConnection)
	// the slot should normally do "delete pbaPayload;"
	m_pRmoConnectionPrivate->dispatchPayload(pbaPayload, iType);
	// unlock next receive
    emit recvOk();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void RecvThreadWorker::cleanup() {
    // emit
    emit cleanupNonBlocking();
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
void RecvThreadWorker::onCleanup() {
    deleteLater();
}
