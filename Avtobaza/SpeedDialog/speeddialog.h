#ifndef SPEEDDIALOG_H
#define SPEEDDIALOG_H

#include <QtGui/QDialog>
#include <QAbstractSocket>
#include <QDateTime>
#include <QThread>

#include "ui_speeddialog.h"
#include "sender.h"
#include "receiver.h"

class SpeedDialog : public QDialog
{
	Q_OBJECT

public:
	SpeedDialog(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SpeedDialog();
    void clean();

public slots:
	void onConnected(QString sHost,int iPort);
	void onDisconnected();
	void onStart();
	void onStop();
	void onRefresh();
	void onError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);

public:
	Ui::SpeedDialogClass ui;
    QString m_sHost;
	unsigned int m_iPort;
	bool m_bServer;
	int m_iBlockSize_kB; // block size in kB
	bool m_bDebug;
	double m_dSpeed; // kb/sec
    unsigned int m_nThreads; 
    unsigned int m_iRefresh; // refresh period in sec
    double m_avgSpeed;
	bool m_bConnected;
	QRmoConnection * m_pConnection;
	QList<Sender*> m_senders;
	QList<QThread*> m_threads;
    quint64 m_delta;
	quint64 m_nBytesSent;
	Receiver * m_pReceiver;
	QThread * m_pReceiveThread;
	int m_iReceiveDelay;
	QTime m_tStart;
	QTimer m_timer;
};

#endif // SPEEDDIALOG_H
