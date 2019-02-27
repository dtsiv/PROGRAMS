#ifndef QRMOCONNECTION_H
#define QRMOCONNECTION_H

#include <QAbstractSocket>

#include "qrmoconnection_global.h"

class QRmoConnectionPrivate;
//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------
class QRMOCONNECTION_EXPORT QRmoConnection : public QObject
{
	Q_OBJECT

public:
    enum TcpConnectionRoles {
		ServerRole = 0,
		ClientRole = 1
	};

	static const Qt::ConnectionType UniqueBlockingQueuedConnection =
		(Qt::ConnectionType)(Qt::BlockingQueuedConnection|Qt::UniqueConnection);

public:
	// constructor for ClientRole
    QRmoConnection(QString sHost, int iPort, bool bDebugProto=false, 
        TcpConnectionRoles connectionRole=ClientRole, QObject *parent=0);
	// constructor for ServerRole
    QRmoConnection(int iPort, bool bDebugProto=false, 
		TcpConnectionRoles connectionRole=ServerRole,
	    QString sHost="0.0.0.0", QObject *parent=0);
    ~QRmoConnection();
	// last error code
	QAbstractSocket::SocketError error();
	// last error string
	QString errorString();
	// send pbaPayload over network
	bool send(
		QByteArray *pbaPayload, 
		int iType, 
		unsigned long timeout=ULONG_MAX);
	// getter for (socket_state == QAbstractSocket::ConnectedState)
    bool isConnected();
	// release all waiting send threads
	void doDisconnect();
	// reconnect to another host/port
    void reconnect(QString sHost, int iPort);
	// block user thread until disconnected() signal or !isConnected()
	bool waitForDisconnected(unsigned long iTimeout=-1);


protected:
    // to enable private class hierarchy
    QRmoConnection(QRmoConnectionPrivate /*must be &&*/ &d,
                   QObject *parent);
    // pointer to implementation
    QRmoConnectionPrivate * const d_ptr;

signals:
	// buffer arrived
	void receive(QByteArray* /*pbaPayload*/, int /*iType*/);
	// connected
    void connected(QString /*sHost*/, int /*iPort*/);
	// disconnected
    void disconnected();
	// signal error to the user
	void error(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);

private:
    Q_DECLARE_PRIVATE(QRmoConnection)
    Q_DISABLE_COPY(QRmoConnection)
};

#endif // QRMOCONNECTION_H
