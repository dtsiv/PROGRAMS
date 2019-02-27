#include "receiver.h"
#include <QThread>

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
Receiver::Receiver(QObject *parent, int iDelay)
  : QObject(parent)
        , m_nDelayMs(1000*iDelay)
        , m_bStop(false)
        , m_iSize(0)
        , m_iTotalSize(0) {
    QObject::connect(this,SIGNAL(doFinish()),SLOT(onDoFinish()));
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Receiver::onReceive(QByteArray* ba,int iType) {

	quint64 iSize = ba->size();

	m_iSize += iSize;
	m_iTotalSize += iSize;

	// it is receiver responsibility to delete the QByteArray
	delete ba;

    // qDebug() << "Received: " << iSize;
	if (m_bStop) return;

	if (m_nDelayMs) {
		QMutexLocker locker(&m_mutex);
		m_condition.wait(&m_mutex,m_nDelayMs);
	}
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Receiver::finish() {
    m_bStop = true;
	m_condition.wakeAll();
    emit doFinish();
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
void Receiver::onDoFinish() {
    deleteLater();
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
Receiver::~Receiver() {
    QObject::thread()->quit();
}
