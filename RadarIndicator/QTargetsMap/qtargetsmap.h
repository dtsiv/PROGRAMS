#ifndef QTARGETSMAP_H
#define QTARGETSMAP_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>
#include <QOpenGLWidget>

#include "qproppages.h"
#include "qformular.h"
#include "qtargetmarker.h"
#include "mapwidget.h"

#define QTARGETSMAP_PROP_TAB_CAPTION    "Indicator grid"
#define QTARGETSMAP_DOC_CAPTION         "Targets map"

#define QTARGETSMAP_SCALE_D             "PixelsPerDistanceM"
#define QTARGETSMAP_SCALE_V             "PixelsPerVelMpS"
#define QTARGETSMAP_VIEW_D0             "DistanceOriginM"
#define QTARGETSMAP_VIEW_V0             "VelOriginMpS"

class QTargetsMap : public QObject {
	Q_OBJECT

public:
    QTargetsMap(QWidget *pOwner = 0);
	~QTargetsMap();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);
    void zoomMap(bool bZoomAlongD, bool bZoomAlongV, bool bZoomIn = true);
    void zoomMap(MapWidget *pMapWidget, int iX, int iY, bool bZoomIn = true);

private slots:
    void onExceptionDialogClosed();
    void onFormularTimeout();

public:
    MapWidget *getMapInstance();
    void addTargetMarker(QTargetMarker* pTargetMarker);

signals:
    void doUpdate();

private:
    void mapPaintEvent(MapWidget *pMapWidget, QPaintEvent *qpeEvent);
    bool drawGrid(MapWidget *pMapWidget, QPainter &painter);
    void shiftView(bool bVertical, bool bPositive);
    void shiftView(QPoint qpShift);
    void resetScale();

    struct GridSafeParams {
        double dScaleD; // pixels per m
        double dScaleV; // pixels per m/s
        double dViewD0; // localtion of widget (0,0) along physical D axis (m)
        double dViewV0; // localtion of widget (0,0) along physical V axis (m/s)
    } *m_pSafeParams;

    // scales over horizontal and vertical direction
    double m_dScaleD; // pixels per m
    double m_dMaxScaleD; // maximum scale over distance
    double m_dScaleV; // pixels per m/s
    double m_dMaxScaleV; // maximum scale over velocity
    // the only reference values are coordinates of widget (0,0) along dimensional axes
    double m_dViewD0; // along distance axis (m)
    double m_dViewV0; // along velocity axis (m/s)
    QTransform m_transform; // from physical coordinates to widget coordinates
    QString m_qsLastError;
    QWidget *m_pOwner;

public:
    // last position & time
    QFormular::sMouseStillPos *m_pMouseStill;
    // time delay to show formular
    static const quint64 m_iFormularDelay;

private:
    QTimer m_qtFormular;
    QList<QFormular*> m_qlFormulars;
    QList<QTargetMarker*> m_qlTargets;

    friend class MapWidget;
};



#endif // QTARGETSMAP_H
