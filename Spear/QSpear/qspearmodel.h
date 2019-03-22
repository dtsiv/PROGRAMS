#ifndef QSPEARMODEL_H
#define QSPEARMODEL_H

#include <QObject>
#include <QDateTime>
#include <QSettings>
#include <QMap>
#include <QFile>
#include <QTimer>

#include "qserial.h"
#include "codogramsa.h"

#define SETTINGS_KEY_SRV_ADDRESS                "serverAddress"
#define SETTINGS_KEY_SRV_PORT                   "serverPort"
#define SETTINGS_KEY_FREQUENCY                  "frequency"
#define SETTINGS_KEY_CANNON_AZIM                "cannonAzimuth"
#define SETTINGS_KEY_CANNON_ELEV                "cannonElevation"
#define SETTINGS_KEY_ANT_AZIM                   "remoteAntennaAzimuth"
#define SETTINGS_KEY_ANT_ELEV                   "remoteAntennaElevation"
#define SETTINGS_KEY_BLH_FILENAME               "coordinateFileName"
#define SETTINGS_KEY_IPCAM_URL                  "videoCameraURL"
#define SETTINGS_KEY_GEOMETRY                   "geometry"
#define SETTINGS_KEY_ROT_SPEED                  "rotationSpeed"

#define TIMEOUT_STALE_STATUS                    15000

class QSpear;

class QSpearModel : public QObject {
	Q_OBJECT
public:
	// "changed" flags for model
	enum ChangeFlags {
		flNone        = 0x00000000,
		flConnected   = 0x00000001,
		flCTRL        = 0x00000002,
		flMODE        = 0x00000004,
		flAntSens     = 0x00000008,
		flAntPos      = 0x00000010,
		flIni         = 0x00000020,
		flRotate      = 0x00000040,
		flStatus      = 0x00FFFF00,
		flUD01        = 0x00000100,
		fl4PP01_1     = 0x00000200,
		fl4PP01_2     = 0x00000400,
		fl4PP01_3     = 0x00000800,
		fl4PP01_4     = 0x00001000,
		fl3PP01       = 0x00002000,
		flCS01_1      = 0x00004000,
		flCS01_2      = 0x00008000,
		flGG01        = 0x00010000,
		flSU01        = 0x00020000,
		flTgr         = 0x00040000,
		flEmi         = 0x00080000,
		flRdy         = 0x00100000,
		flEnT         = 0x00200000,
		flAll         = 0xFFFFFFFF
	};

	enum HardwareStatus {
		stUndefined   = 0x00000000,
		stNoError     = 0x00000001,
		stError       = 0x00000100
	};

public:
	// public static members and fields
    static double dMinFreq,dMaxFreq,dFreqStep;
    static double dMinAzimuth,dMaxAzimuth;
    static double dMinElevation,dMaxElevation;
	static QString placeholderFreq,placeholderAzim,placeholderElev;
	static QString formatFreqString(QString qsFreq);

public:
	QSpearModel(QSpear *pOwner, QObject *parent=0);
	~QSpearModel();

    bool readSettings();
    bool readCoordinates();
    bool writeCoordinates();
	void startWork(bool bStart=true);

	// setters-getters
	QByteArray*         getCodogramByType(int iType);
	bool                recvCodogram(int iType, QByteArray *pba);
        QString             workTime(bool bTotal=false);
    bool                isConnected();
    bool                bInitError() { return m_bInitError; }
    void                setConnected(bool);
    bool                setCoordinates(QStringList qslCoord);
    QStringList         getCoordinates();
    void                setIrradiation(bool);
    void                svernuti();
    void                startFK();
    void                startStrelba();
    QString             qsFreq();
    void                setFreq(QString qsFreq);
    int                 rotSpeed() { return m_cgRotate.Speed; }
    void                setRotSpeed(int iSpeed) { m_cgRotate.Speed=iSpeed; }
    void                setRotDir(int iDir);
    void                setCannonDir();
    void                setASysDir();
	QString             qsAzim();
	QString             qsElev();

    QString m_qsErrorMessage;
signals:
	void changed(QSpearModel::ChangeFlags fWhat);

private slots:
    void onStaleStatus();

private:
    QSpear         *m_pOwner;
    QSettings      *m_pSettings;
    QFile          m_qfCoordinates;
	QMap<QString,QVariant> m_qmParamDefaults;
    double         m_dASysLat,m_dASysLon,m_dASysHei,
                   m_dCanLat,m_dCanLon,m_dCanHei;

    // input codograms
    CG_STATUS      m_cgStatus;     //0x4001
	CG_ANT_SENSOR  m_cgAntSensor;  //0x4002

    // output codograms
    CG_CTRL        m_cgCtrl;       //0x2000
	CG_MODE        m_cgMode;       //0x2001
	CG_ANT_POS     m_cgAntPos;     //0x2002
    CG_ROTATE      m_cgRotate;     //0x2003
    CG_INI         m_cgIni;        //0x2004

	QDateTime      m_dtWorkStart;
    quint64  m_uWorkSecsTotal;

	bool           m_bConnected;
    bool           m_bInitError;

    CG_STATUS      m_staleStatus;
    bool           m_bStaleStatus;
    QTimer         m_qtStaleStatus;
};

#endif // QSPEARMODEL_H
