#ifndef QGEOUTILS_H
#define QGEOUTILS_H

#include <QtCore>
#include <QMetaObject>

#include "qproppages.h"
#include "tdcord.h"

#define QGEOUTILS_PROP_TAB_CAPTION     "Geolocation"

#define QGEOUTILS_MAINCTRL                             "MainCtrl"

class QGeoUtils : public QObject {
    Q_OBJECT

public:
    QGeoUtils();
    ~QGeoUtils();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

public slots:
    void onMainctrlChoose();

public:
    BLH m_blhViewPoint;
    BLH m_postBlh[MAX_dwPosCount+1];
    XYZ m_postTc[MAX_dwPosCount+1];

private:
    bool readMainCtrlCfg();

    QString m_qsMainctrlCfg;

};

#endif // QGEOUTILS_H
