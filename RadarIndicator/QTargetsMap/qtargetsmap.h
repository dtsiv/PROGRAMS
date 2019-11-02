#ifndef QTARGETSMAP_H
#define QTARGETSMAP_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>
#include <QOpenGLWidget>

#include "qproppages.h"

#define QTARGETSMAP_PROP_TAB_CAPTION    "Indicator grid"
#define QTARGETSMAP_DOC_CAPTION         "Targets map"

class MapWidget;

class QTargetsMap : public QObject {
	Q_OBJECT

public:
    QTargetsMap(QWidget *pOwner = 0);
	~QTargetsMap();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);
    void zoomMap(bool bZoomAlongD, bool bZoomAlongV, bool bZoomIn = true);

private slots:
    void onExceptionDialogClosed();

public:
    MapWidget *getMapInstance();

signals:
    void doUpdate();

private:
    void mapPaintEvent(MapWidget *pMapWidget, QPaintEvent *qpeEvent);
    bool drawGrid(MapWidget *pMapWidget, QPainter &painter);

    struct GridSafeParams {
        double dScaleD; // pixels per m
        double dScaleV; // pixels per m/s
        double dViewD0; // along distance axis (m)
        double dViewV0; // along velocity axis (m/s)
    } *m_pSafeParams;

    double m_dScaleD; // pixels per m
    double m_dMaxScaleD; // maximum scale over distance
    double m_dScaleV; // pixels per m/s
    double m_dMaxScaleV; // maximum scale over velocity
    // the only reference values are coordinates of widget center along dimensional axes
    double m_dViewD0; // along distance axis (m)
    double m_dViewV0; // along velocity axis (m/s)
    QString m_qsLastError;
    QWidget *m_pOwner;

    friend class MapWidget;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class MapWidget : public QOpenGLWidget {
	Q_OBJECT

public:
    MapWidget(QTargetsMap *owner, QWidget *parent = 0);
	~MapWidget();

protected:
    void paintEvent(QPaintEvent *qpeEvent);
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

private:
    QTargetsMap *m_pOwner;
};


#endif // QTARGETSMAP_H
