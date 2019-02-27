#include "speeddialog.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

SpeedDialog::SpeedDialog(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
	, m_sHost("127.000.000.001")
	, m_iPort(7776)
	, m_bServer(true)
	, m_iBlockSize_kB(4)
	, m_bDebug(true)
	, m_dSpeed(1024.0e0)
	, m_nThreads(10)
	, m_iRefresh(1)
	, m_avgSpeed(0.0e0)
	, m_bConnected(false)
	, m_pConnection(0)
	, m_nBytesSent(0)
	, m_pReceiver(0)
	, m_pReceiveThread(0)
	, m_iReceiveDelay(0)
{
	ui.setupUi(this);
	ui.leHost->setText(m_sHost);
	ui.lePort->setText(QString::number(m_iPort));
	ui.cbServer->setChecked(m_bServer);
	ui.leSize->setText(QString::number(m_iBlockSize_kB));
	ui.cbDebug->setChecked(m_bDebug);
	ui.leSpeed->setText(QString::number(m_dSpeed,'f',0));
	ui.sbThreads->setValue(m_nThreads);
	ui.leRefresh->setText(QString::number(m_iRefresh));
	ui.lbSpeed->setText(QString::number(0.0e0,'e',1));
	ui.leReceiveDelay->setText(QString::number(m_iReceiveDelay));
	ui.lbBytesSent->setText(QString::number(0));

    setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));

	ui.stopButton->setEnabled(false);
	ui.startButton->setEnabled(true);

	QObject::connect(
        &m_timer,
		SIGNAL(timeout()),
		SLOT(onRefresh()));
}

void SpeedDialog::onStart() {
	m_sHost = ui.leHost->text();
	m_iPort = qBound(1,ui.lePort->text().toInt(),65535);
	ui.lePort->setText(QString::number(m_iPort));
	m_bServer = ui.cbServer->isChecked();
	// block size kB
	m_iBlockSize_kB = qBound(1,ui.leSize->text().toInt(),1024*1024);
	ui.leSize->setText(QString::number(m_iBlockSize_kB));
	m_bDebug = ui.cbDebug->isChecked();
	m_dSpeed = ui.leSpeed->text().toDouble();
	m_nThreads = qBound(0,ui.sbThreads->value(),100);
    ui.sbThreads->setValue(m_nThreads);
	m_iRefresh = qBound(1,ui.leRefresh->text().toInt(),60);
	ui.leRefresh->setText(QString::number(m_iRefresh));

	m_iReceiveDelay = qMax(0,ui.leReceiveDelay->text().toInt());
    ui.leReceiveDelay->setText(QString::number(m_iReceiveDelay));

	m_delta = m_iRefresh*1000;
	m_tStart = QDateTime::currentDateTime().time();
	m_avgSpeed = 0.0e0;

	if (m_bServer) {
	    m_pConnection = new QRmoConnection(m_iPort, m_bDebug);
	}
	else {
	    m_pConnection = new QRmoConnection(m_sHost, m_iPort, m_bDebug);
	}
	QObject::connect(
		m_pConnection,
		SIGNAL(connected(QString,int)),
        SLOT(onConnected(QString,int)));
	QObject::connect(
		m_pConnection,
		SIGNAL(disconnected()),
        SLOT(onDisconnected()));
	QObject::connect(
		m_pConnection,
		SIGNAL(error(QAbstractSocket::SocketError, QString)),
        SLOT(onError(QAbstractSocket::SocketError, QString)));

	qDebug() << "m_nThreads=" <<m_nThreads;
#pragma GCC diagnostic ignored "-Wsign-compare"
	for (int i=0; i<m_nThreads; i++) {

		int iType=100+i;

		// qDebug() << "appending sender!" ;
		Sender * pSender = new Sender(0,m_pConnection,m_iBlockSize_kB,m_dSpeed,iType);
		m_senders.append(pSender);

        QThread *pThread = new QThread;
        m_threads.append(pThread);

		pSender->moveToThread(pThread);

		QObject::connect(
			pThread,
			SIGNAL(started()),
			pSender,
			SLOT(onStarted()));

		// qDebug() << "starting thread";

		pThread->start();

	}
#pragma GCC diagnostic pop


	ui.stopButton->setEnabled(true);
	ui.startButton->setEnabled(false);

	// receiver 
	m_pReceiver = new Receiver(0,m_pConnection,m_iReceiveDelay);
	m_pReceiveThread = new QThread;
	m_pReceiver->moveToThread(m_pReceiveThread);
    // receive slot
	QObject::connect(
		m_pConnection,
		SIGNAL(receive(QByteArray*,int)),
		m_pReceiver,
        SLOT(onReceive(QByteArray*,int)),
		QRmoConnection::UniqueBlockingQueuedConnection);
	m_pReceiveThread->start();

	// clear old info
	ui.plainTextEdit->clear();

	// start refresh timer
	m_timer.start(m_delta);
}

void SpeedDialog::onStop() {
    // stop the timer
	qDebug() << "before m_timer.stop()";
	m_timer.stop();

	// delete dynamic members
    clean();

	ui.lbSpeed->setText(QString::number(0.0e0,'e',1));

	ui.stopButton->setEnabled(false);
	ui.startButton->setEnabled(true);
}

void SpeedDialog::clean() {
	// release all waiting send() calls
	qDebug() << "m_bServer=" << m_bServer << " entered clean()";
	if (m_pConnection) { 
		qDebug() << "before doDisconnect()";
		m_pConnection->doDisconnect();
		qDebug() << "before waitForDisconnected()";
		qDebug() << "m_bServer=" << m_bServer
			     << " waitForDisconnected (1 sec)=" 
				 << m_pConnection->waitForDisconnected(1000);
	}

	// qDebug() << "After disconnect";
    // if there are senders - cleanup
	if (m_senders.count()) {
		while (!m_senders.isEmpty()) {
			Sender * pSender = m_senders.takeLast();
            // qDebug() << "Calling finish";
		    qDebug() << "before pSender->finish()";
			pSender->finish();
			// qDebug() << "Calling waitForFinished";
			if (!pSender->waitForFinished(1000)) qDebug() << "pSender->waitForFinished(1sec)=false";
			delete pSender;
		}
		while (!m_threads.isEmpty()) {
			QThread * pThread = m_threads.takeLast();
			// if (pThread->isFinished()) qDebug() << "Thread finished";
			if (pThread->isRunning())  {
                pThread->quit();
				if (!pThread -> wait(1000)) qDebug() << "pThread->wait(1sec)=false";
			}
			delete pThread;
		}
	}
	// cleanup receiver
	if (m_pReceiver) {
		m_pReceiver->wake();
		delete m_pReceiver;
	}
	m_pReceiver = 0;
    // receiver thread
	if (m_pReceiveThread) {
		if (m_pReceiveThread->isRunning())  {
            m_pReceiveThread->quit();
			if (!m_pReceiveThread -> wait(1000)) qDebug() << "m_pReceiveThread->wait(1sec)=false";
		}
        delete m_pReceiveThread;
	}
	m_pReceiveThread = 0;
	// qDebug() << "delete m_pConnection: ...";
	if (m_pConnection) { 
	    qDebug() << "before delete m_pConnection";
		delete m_pConnection;
	    qDebug() << "delete m_pConnection: ok";
	}
    m_pConnection = 0;
	qDebug() << "m_bServer=" << m_bServer << " clean() finished";
}

void SpeedDialog::onRefresh() {

	if (!m_bConnected) return;
	if (!m_pReceiver) return;

	quint64 interval = m_tStart.msecsTo(QDateTime::currentDateTime().time());

	m_avgSpeed = 1.0e3*m_pReceiver->m_iSize*8/(1024*1024)/interval;
	m_tStart = QDateTime::currentDateTime().time();
	m_pReceiver->m_iSize = 0;

	quint64 nBytesSent=0;
	if (m_senders.count()) {
		QList<Sender*>::iterator i;
		for (i=m_senders.begin(); i!=m_senders.end(); ++i) {
			nBytesSent+=(*i)->m_iSent_total;
		}
	}
	ui.lbBytesSent->setText(QString("%1 %3 (%2 %4)")
		.arg(nBytesSent)
		.arg(1.0e-3*qRound(1.0e3*nBytesSent/1024.0e0/1024.0e0))
		.arg(QString::fromLocal8Bit("а"))
		.arg(QString::fromLocal8Bit("ла")));

	ui.lbSpeed->setText(QString::number(m_avgSpeed,'e',1));
	ui.plainTextEdit->appendPlainText(
			QString("Received %1 bytes (%2 MB)")
			.arg(m_pReceiver->m_iTotalSize)
			.arg(1.0e0*m_pReceiver->m_iTotalSize/(1024*1024)));
}

void SpeedDialog::onError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString) {
	ui.plainTextEdit->appendPlainText(qsErrorString);
}

void SpeedDialog::onDisconnected() {
	m_bConnected = false;
    ui.plainTextEdit->clear();
	ui.plainTextEdit->appendPlainText("Disconnected");
}

void SpeedDialog::onConnected(QString sHost,int iPort) {
	m_bConnected = true;
	ui.plainTextEdit->appendPlainText(QString("Connected to host %1 port %2.").arg(sHost).arg(iPort));
}

SpeedDialog::~SpeedDialog() {
	clean();
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
