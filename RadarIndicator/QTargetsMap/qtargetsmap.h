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
    QTargetsMap();
	~QTargetsMap();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

    MapWidget *getMapInstance();

private:
    void mapPaintEvent(MapWidget *pMapWidget, QPaintEvent *qpeEvent);
    void drawGrid(MapWidget *pMapWidget, QPainter &painter);

    friend class MapWidget;
};

class MapWidget : public QOpenGLWidget {
	Q_OBJECT

public:
    MapWidget(QTargetsMap *owner, QWidget *parent = 0);
	~MapWidget();

protected:
    void paintEvent(QPaintEvent *qpeEvent);
    void initializeGL() {
              // Set up the rendering context, load shaders and other resources, etc.:
              QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
              f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
              // ...
    }

private:
    QTargetsMap *m_pOwner;
};


#endif // QTARGETSMAP_H
