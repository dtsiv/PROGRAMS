#include "recvthreadworker.h"

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
RecvThreadWorker::RecvThreadWorker(QRmoConnectionPrivate * pRmoConnectionPrivate)
        : QObject(0)
        , m_pRmoConnectionPrivate(pRmoConnectionPrivate)
{
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
RecvThreadWorker::~RecvThreadWorker() { }

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
