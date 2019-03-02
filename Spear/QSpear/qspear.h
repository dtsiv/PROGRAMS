#ifndef QSpear_H
#define QSpear_H

#include <QtGui>
#if QT_VERSION >= 0x050100
#include <QtWidgets>
#endif
#include <QMainWindow>

#include "ui_qspear.h"
#include "rmoexception.h"
#include "qexceptiondialog.h"
#include "qledindicator.h"
#include "qrmoconnection.h"
#include "qspearmodel.h"

#define QSPEAR_VER_MAJOR                  2
#define QSPEAR_VER_MINOR                  3

class VlcWidgetVideo;
class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

class QRmoLineEdit;
class QRmoValidator;

class QSpear : public QMainWindow
{
	Q_OBJECT

public:
    QSpear(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~QSpear();

public slots:
	void onAbout();
	void onStartup();
	void onCommand();
	void onWorkTimeout();

    void onConnected(QString sHost,int iPort);
    void onDisconnected();
    void onError(QAbstractSocket::SocketError iErrorCode, QString qsErrorString);
    void onReceive(QByteArray*,int);

	void throwException();

protected slots:
	void onModelChanged(QSpearModel::ChangeFlags fWhat);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
	void arrangeControls();
    void indicateIrr(bool);
    void indicateReady(bool);

	void adjustLabel(QLabel *pLabel, int iPointSize, 
		QString qsMask=QString(),
		Qt::Alignment alignment=Qt::AlignHCenter|Qt::AlignVCenter);

    // helper functions - group box layouts
    QLayout *helperLayoutTopIndicators();
    QLayout *helperLayoutChastota();
    QLayout *helperLayoutRezhimRaboty();
    QLayout *helperLayoutControli();
    QLayout *helperLayoutUpravlenie();
    QLayout *helperLayoutPolozhenieAS();
    QLayout *helperLayoutCoordinaty();
    QLayout *helperLayoutJustirovka();

	Ui::QSpearClass ui;

	QAction *exitAct;
    QAction *aboutAct;
	QAction *adjustAct;

	QLabel *lbStatusConn;

	QLedIndicator *m_pledReady;
	QLedIndicator *m_pledIrradiation;
	QLedIndicator *m_pledAntennaSys;
	QLedIndicator *m_pledUD01;
	QLedIndicator *m_pled4PP01_1;
	QLedIndicator *m_pled4PP01_2;
	QLedIndicator *m_pled4PP01_3;
	QLedIndicator *m_pled4PP01_4;
	QLedIndicator *m_pled3PP01;
	QLedIndicator *m_pledCS01_1;
	QLedIndicator *m_pledCS01_2;
	QLedIndicator *m_pledGG01;
	QLedIndicator *m_pledSU01;
	QLedIndicator *m_pledTemperature;

	QRmoLineEdit  *m_pleFreq;
	QRmoLineEdit  *m_pleCannonB;
	QRmoLineEdit  *m_pleCannonL;
	QRmoLineEdit  *m_pleCannonH;
	QRmoLineEdit  *m_pleAntB;
	QRmoLineEdit  *m_pleAntL;
	QRmoLineEdit  *m_pleAntH;

	QLabel        *m_pqlWorkTime;
	QLabel        *m_pqlStatusMsg;
    QLabel        *m_pqlASAzim;
    QLabel        *m_pqlASElev;
    QLabel        *m_pqlIrradiation;
    QLabel        *m_pqlReady;
    QLabel        *m_pqlRotSpeed;

	QPlainTextEdit     *m_pteConsole;

	QPushButton   *m_ppbIrradiation;
	QPushButton   *m_ppbApplyFreq;
	QPushButton   *m_ppbRazvernuti;
	QPushButton   *m_ppbFK;
	QPushButton   *m_ppbPrepareData;
	QPushButton   *m_ppbStrelba;
	QPushButton   *m_ppbObrabotka;
	QPushButton   *m_ppbSvernuti;
    QPushButton   *m_ppbRotLeft;
    QPushButton   *m_ppbRotRight;
    QPushButton   *m_ppbRotUp;
    QPushButton   *m_ppbRotDown;
    QPushButton   *m_ppbRotStop;
    QPushButton   *m_ppbZoomIn;
    QPushButton   *m_ppbZoomOut;
    QPushButton   *m_ppbSaveDirAS;
    QPushButton   *m_ppbSaveDirCan;
    QPushButton   *m_ppbExit;
    QPushButton   *m_ppbSaveCoord;
    QPushButton   *m_ppbClearMessages;

    QSlider       *m_pslRotationSpeed;

	QGroupBox     *m_pgbRezhimRaboty;
	QGroupBox     *m_pgbUpravlenie;
	QGroupBox     *m_pgbKontroli;
	QGroupBox     *m_pgbFrequency;
	QGroupBox     *m_pgbCoordinates;
	QGroupBox     *m_pgbPolozhenieAS;
	QGroupBox     *m_pgbExit;
    QGroupBox     *m_pgbJustirovka;

    QVBoxLayout    *m_vblLeftColumn;
    QVBoxLayout    *m_vblCentralColumn;
    QVBoxLayout    *m_vblRightColumn;
    QHBoxLayout    *m_hblAllColumns;

	QTranslator    *m_pappTranslator;
	QRmoConnection *m_pConnection;

    QList<QWidget*> m_qlMainControls,m_qlWin1Controls;

    // parameters values
	QString        m_qsServerAddress;
	int            m_iServerPort;

	QString        m_qsIPCamURL;

	QSpearModel    *m_pModel;
	QTimer         m_workTimer;
    QString        m_qsErrorMessage;

    VlcWidgetVideo *m_pVideo;
    VlcInstance *m_pInstance;
    VlcMedia *m_pMedia;
    VlcMediaPlayer *m_pPlayer;

    friend class QSpearModel;
    friend class UserControlInputFilter;
};
//*****************************************************************************
//
//*****************************************************************************
class UserControlInputFilter : public QObject {
protected:
	virtual bool eventFilter(QObject*, QEvent*);

public:
    UserControlInputFilter(QSpear* pOwner, QObject *pobj=0);

private:
	QSpear *m_pOwner;
};
//*****************************************************************************
//
//*****************************************************************************
class QRmoLineEdit : public QLineEdit {
	Q_OBJECT
public:
	enum ContentTypes {
		ctFixedPoint     =0x100,
		ctInteger        =0x200,
		ctFrequency      =0x101,
		ctAzimuth        =0x102,
		ctElevation      =0x103,
		ctArbitrary      =0x1000};
	QRmoLineEdit(const QString & contents, 
		ContentTypes type = ctArbitrary,
		QWidget * parent = 0); 
protected slots:
	void onEditingFinished();
    void onFreqEditingFinished();

protected:
	virtual void focusInEvent ( QFocusEvent * event );
};

//*****************************************************************************
//
//*****************************************************************************
class QRmoValidator : public QRegExpValidator {
	Q_OBJECT
public:
	QRmoValidator (const QRegExp &rx, QString qsDefault, QObject *parent)
		:QRegExpValidator (rx, parent), m_qsDefault(qsDefault) {}
	void fixup (QString &input) const;
private:
	QString m_qsDefault;
};

#endif // QSpear_H

