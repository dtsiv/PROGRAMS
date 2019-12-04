#ifndef QPOSTSVIEW_H
#define QPOSTSVIEW_H

#include <QtWidgets>
#include <QObject>

#include "qavtctrl.h"
#include "codograms.h"

class QPostsView : public QGraphicsView {
    Q_OBJECT
public:
    explicit QPostsView(QString qsMainctrlCfg, QWidget *parent = 0);

signals:

public slots:
    void onMainctrlChanged(QString qsMainctrlCfg);
    void onMainctrlChoose();

private:
    QGraphicsScene m_scene;
};

#endif // QPOSTSVIEW_H
