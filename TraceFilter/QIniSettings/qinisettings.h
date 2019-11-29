#ifndef QINISETTINGS_H
#define QINISETTINGS_H

#include <QtCore>
#include <QSettings>
#include "qserial.h"

#define RADAR_INDICATOR_INI_FILE                 "TraceFilter.xml"

class QIniSettings {
public:
    enum STATUS_CODES {
        READ_VALID      = 0,
        READ_DEFAULT    = 1,
        KEY_NOT_FOUND   = 2,
        INIT_ERROR      = 3
    };
	
// Singleton construction for QIniSettings class
private: 
    QIniSettings() {}
    ~QIniSettings();
    QIniSettings( const QIniSettings&);
    QIniSettings& operator=( QIniSettings& );

public:
    static QIniSettings& getInstance();

// payload part of the class
private:

    static void openIniSource();

    static bool bInit;
    static QSettings *m_pSettings;
    static QString m_qsIniFileName;
    static QMap<QString,QVariant> m_qmParamDefaults;

public:
    QVariant value(const QString &key, STATUS_CODES &iStatus);
    void setDefault(const QString &key, const QVariant &value);
    void setDefault(const QMap<QString,QVariant> &qmNewDefaults);
    void setValue(const QString &key, const QVariant &value);
    QVariant getDefault(const QString &key);
};

#endif // QINISETTINGS_H
