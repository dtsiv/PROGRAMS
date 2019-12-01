#ifndef QGEOUTILS_H
#define QGEOUTILS_H

#include <QtCore>
#include <QMetaObject>

#include "qproppages.h"

#define QGEOUTILS_PROP_TAB_CAPTION     "Geolocation"

#define QGEOUTILS_MAINCTRL                             "MainCtrl"

class QGeoUtils : public QObject {
    Q_OBJECT

public:
    QGeoUtils();
    ~QGeoUtils();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

private:
    QString m_qsMainctrlCfg;

};

#endif // QGEOUTILS_H