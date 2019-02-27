#include "sender.h"

Sender::Sender(QObject *parent
		, QRmoConnection * pConnection
		, int iBlockSize //kB
		, double dSpeed // kb/s
		, int iType
			   )
	: QObject(parent)
	    , m_pConnection(pConnection)
	    , m_pTimer(0)
		, m_iType(iType)
		, m_bStop(false)
		, m_pbaBuf(0)
{
	// block size in Bytes!
    m_iBlockSizeBytes = 1024*qMax(iBlockSize,1);
    dSpeed = qBound(1.0e0,dSpeed,1.0e9);

	// create payload buffer
	m_pbaBuf = new QByteArray(m_iBlockSizeBytes,0);
	if (!m_pbaBuf) {
		qDebug() << "Could not create payload!";
		return;
	}
	m_nBufSeqNum = 0;
	unsigned int * pBufNum = (unsigned int *)m_pbaBuf->data();
	pBufNum[0] = m_nBufSeqNum;
	for (int i=4; i<m_iBlockSizeBytes; i++) (*m_pbaBuf)[i] = (rand()>>7);

	QObject::connect(this,SIGNAL(doFinish()),SLOT(onDoFinish()));
	// calculate interval in msec for send timer
	m_iIntervalMsec = qRound(
		   1.0e0 
		/ ( dSpeed /*required speed in kb/sec*/ 
		   *1024.0e0 /*bits per kb*/
		   /8.0e0 /*bits per Byte*/ )
		* m_iBlockSizeBytes /*Bytes*/
		* 1000.0e0 /*msec*/ );
	// protect from zero interval
	m_iIntervalMsec = qMax(m_iIntervalMsec,1);
}

void Sender::finish() {
	emit doFinish();
}

// connected to QThread's started signal
void Sender::onStarted() {
    m_pTimer = new QTimer;
	QObject::connect(m_pTimer,SIGNAL(timeout()),SLOT(onTimeout()));
	m_pTimer->start(m_iIntervalMsec);
	m_tLast = QDateTime::currentDateTime().time();
    m_iSent = 0;
    m_iSent_total = 0;
}

void Sender::onTimeout() {
	if (m_bStop) qDebug() << "onTimeout";
	// no further operation after stop signal
	// QMutexLocker locker(&m_mutex);
    if (m_bStop) return;
    // cannot send over disconnected socket
	if (!m_pConnection || !m_pConnection->isConnected()) { 
		// qDebug() << "no connection!";
		return;
	}
	// send with infinite timeout (and stop on error)
	bool bRet = m_pConnection->send(m_pbaBuf, m_iType);
	if (!bRet) {
		qDebug() 
			<< "send() returned false. size, error=" 
			<< m_pbaBuf->size() 
			<< m_pConnection->errorString();
        m_pTimer->stop();
		return;
	}
	m_iSent += m_pbaBuf->size();
	// add to the total
	m_iSent_total += m_pbaBuf->size();
	// time in seconds
	double dElapsed = 1.0e-3 * m_tLast.msecsTo(QDateTime::currentDateTime().time());
	if (dElapsed > 3.0e0) {
//		qDebug() << " Sent " << m_iSent_total << " bytes. Speed(kb/s)=" << m_iSent*8 / dElapsed / 1024;
		m_iSent=0;
        m_tLast=QDateTime::currentDateTime().time();
	}
	// generate new random data
	char * pBuf = m_pbaBuf->data();
	// for (int i=0; i<m_iBlockSizeBytes; i++) (*m_pbaBuf)[i] = (rand()>>7);
	for (int i=0; i<m_iBlockSizeBytes; i++) pBuf[i] = (rand()>>7);
}

void Sender::onDoFinish() {
	// QMutexLocker locker(&m_mutex);
	if (m_pTimer) { 
		m_pTimer->stop();
		delete m_pTimer;
		m_pTimer = 0;
	}
	m_waitCondition.wakeAll();
    m_bStop = true;
}

bool Sender::waitForFinished(int timeout) {
	QMutexLocker locker(&m_mutex);
    if (m_bStop) return true;
	return m_waitCondition.wait(&m_mutex,timeout);
}

Sender::~Sender() {
	// can only be called from this thread
	if (m_pbaBuf) delete m_pbaBuf;
}
