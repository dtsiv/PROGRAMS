#ifndef QFORMULAR_H
#define QFORMULAR_H

#include <QObject>
#include <QTime>
#include <QPoint>
#include <QPainter>
#include <QRect>

#include "qtargetmarker.h"

class MapWidget;

class QFormular : public QObject
{
    Q_OBJECT
public:
    struct sMouseStillPos;
public:
    explicit QFormular(struct sMouseStillPos *pMouseStill,
                       // scales over horizontal and vertical direction
                       double dScaleD, // pixels per m
                       double dScaleV, // pixels per m/s
                       // the only reference values are coordinates of widget center along dimensional axes
                       double dViewD0, // along distance axis (m)
                       double dViewV0, // along velocity axis (m/s)
                       QObject *parent = 0);

    void drawFormular(MapWidget *pMapWidget,
                      // scales over horizontal and vertical direction
                      double dScaleD, // pixels per m
                      double dScaleV, // pixels per m/s
                      // the only reference values are coordinates of widget center along dimensional axes
                      double dViewD0, // along distance axis (m)
                      double dViewV0, // along velocity axis (m/s)
                      QPainter &painter);
    bool selectTarg(QList<QTargetMarker*> ql_Targets);
    void setStale();
    bool isStale();
    static const quint64 m_uFormularLifetime;
    static const quint64 m_uFormularCutoffPix;

    struct sMouseStillPos {
        QTime last;
        QPoint pos;
        QRect geometry;
    } m_mouseStill;

signals:

public slots:

private:
    // physical coordinates of mouse pointer
    double m_dMouseDPhys; // physical coordinate D (m) of mouse pointer
    double m_dMouseVPhys; // physical coordinate V (m/s) of mouse pointer
    // scales over horizontal and vertical direction
    double m_dScaleD; // pixels per m
    double m_dScaleV; // pixels per m/s
    // the only reference values are coordinates of widget center along dimensional axes
    double m_dViewD0; // along distance axis (m)
    double m_dViewV0; // along velocity axis (m/s)
    // pointer to QTargetMarker (if any)
    QTargetMarker *m_pTargetMarker;
    bool m_bStale;
};

#endif // QFORMULAR_H
