#include "receiver.h"

Receiver::Receiver(QObject *parent
	, QRmoConnection * pConnection
	, int iDelay)
  : QObject(parent)
	, m_pConnection(pConnection)
	, m_nDelayMs(1000*iDelay)
    , m_bStop(false)
    , m_iSize(0)
    , m_iTotalSize(0)
{

}


void Receiver::onReceive(QByteArray* ba,int iType) {

	quint64 iSize = ba->size();

	m_iSize += iSize;
	m_iTotalSize += iSize;

	// it is receiver responsibility to delete the QByteArray
	delete ba;

	if (m_bStop) return;

	if (m_nDelayMs) {
		QMutexLocker locker(&m_mutex);
		m_condition.wait(&m_mutex,m_nDelayMs);
	}
}

void Receiver::wake() {
    m_bStop = true;
	m_condition.wakeAll();
}

Receiver::~Receiver()
{
	// protect m_condition from being deleted while thread is waiting
    QMutexLocker locker(&m_mutex);
}
