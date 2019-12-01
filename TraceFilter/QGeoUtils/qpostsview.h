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
    void getViewPoint(PBLH pblhViewPoint, PMAINCTRL pMainCtrl);

    QGraphicsScene m_scene;
    QString m_qsMainctrlCfg;
    BLH m_blhViewPoint;
};

#endif // QPOSTSVIEW_H
