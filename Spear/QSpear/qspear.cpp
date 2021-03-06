#include "qspear.h"
#include "qserial.h"
#include "codogramsa.h"
#include <QtGlobal>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QTranslator>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSpear::QSpear(QWidget *parent, Qt::WindowFlags flags)
		: QMainWindow(parent, flags /* | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint */ ) 
		, exitAct(NULL)
		, aboutAct(NULL)
		, lbStatusConn(NULL)
		, m_pConnection(NULL)
		, m_qsServerAddress("127.0.0.1")
		, m_iServerPort(2001)
		, m_pModel(NULL) 
		, m_pVideo(NULL)
		, m_pInstance(NULL)
		, m_pMedia(NULL)
		, m_pPlayer(NULL)
{
    // basi UI setup
	ui.setupUi(this);
    setWindowIcon(QIcon(QPixmap(":/Resources/spear.ico")));
    // setWindowState( Qt::WindowFullScreen);
    // setWindowState(Qt::WindowMaximized);

	// initialize model
	m_pModel = new QSpearModel(this);
    if (!m_pModel->readSettings()) {
        m_qsErrorMessage=QString("Init failed: ")+m_pModel->m_qsErrorMessage;
        QTimer::singleShot(0,this,SLOT(throwException()));
        // return;
    }
    if (!m_pModel->readCoordinates()) {
        m_qsErrorMessage=QString("Read coordinates failed: ")+m_pModel->m_qsErrorMessage;
        QTimer::singleShot(0,this,SLOT(throwException()));
        // return;
    }

    qRegisterMetaType<QSpearModel::ChangeFlags>("QSpearModel::ChangeFlags");
	QObject::connect(m_pModel,SIGNAL(changed(QSpearModel::ChangeFlags)),
		                      SLOT(onModelChanged(QSpearModel::ChangeFlags)));
 	arrangeControls();

    // controls of the two windows
    m_qlMainControls << m_ppbIrradiation << m_ppbRazvernuti << m_ppbFK  << m_ppbPrepareData
         << m_ppbStrelba << m_ppbObrabotka << m_ppbSvernuti;
    m_qlWin1Controls << m_pgbJustirovka << m_pgbExit << m_pgbCoordinates << m_pgbFrequency;

	// TCP connection
	m_pConnection=new QRmoConnection(m_qsServerAddress,m_iServerPort);
	QObject::connect(m_pConnection,
		SIGNAL(connected(QString,int)),
        SLOT(onConnected(QString,int)));
	QObject::connect(m_pConnection,SIGNAL(disconnected()),SLOT(onDisconnected()));
	QObject::connect(
		m_pConnection,
		SIGNAL(error(QAbstractSocket::SocketError, QString)),
        SLOT(onError(QAbstractSocket::SocketError, QString)));
	QObject::connect(
		m_pConnection,
		SIGNAL(receive(QByteArray*,int)),
        SLOT(onReceive(QByteArray*,int)));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QSpear::~QSpear() {
    delete m_pModel;
	if (m_pConnection) {
		m_pConnection->doDisconnect();
		qDebug() << "before waitForDisconnected()";
        bool bWait = m_pConnection->waitForDisconnected(30000);
		qDebug() << "waitForDisconnected(30000)=" << bWait;
        // ::Sleep(10000);
        if (bWait) delete (m_pConnection);
	}
    // QProcess::execute("shutdown",QStringList("/i"));
    // if (m_pInstance) delete m_pInstance;
    // if (m_pMedia) delete m_pMedia;
    // if (m_pPlayer) delete m_pPlayer;
    // if (m_pVideo) delete m_pVideo;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::adjustLabel(QLabel *pLabel, int iPointSize, 
		QString qsMask /* =QString() */,
		Qt::Alignment alignment /* =Qt::AlignHCenter|Qt::AlignVCenter */ ) {
	QFont qfo;
	qfo.setPointSize(iPointSize);
	pLabel->setFont(qfo);
	if (!qsMask.isEmpty()) pLabel->setFixedWidth(pLabel->fontMetrics().width(qsMask));
	pLabel->setAlignment(alignment);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::throwException() {
    throw RmoException(QString("Error: "+m_qsErrorMessage));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSpear::closeEvent(QCloseEvent *event) {
    event->accept();
}
