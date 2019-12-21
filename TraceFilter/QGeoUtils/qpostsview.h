#ifndef QPOSTSVIEW_H
#define QPOSTSVIEW_H

#include <QtWidgets>
#include <QObject>

#include "qavtctrl.h"
#include "codograms.h"

class QPostsView : public QWidget {
    Q_OBJECT
public:
    explicit QPostsView(QString qsMainctrlCfg, QWidget *parent = 0);
    virtual QSize sizeHint() const;
    bool onMainctrlChanged(QString);

protected:
    virtual void paintEvent(QPaintEvent *qpeEvent);

public:
    QString m_qsMainctrlCfg;
    PMAINCTRL m_pMainCtrl;
    BLH m_blhViewPont;

private:
    int m_iLabelSize;
    int m_iPenWidth;
    int m_iMargin;
    QList<int> m_qlPostIds;
    QList<QPoint> m_qlPosts;
    int m_iWgtSz;
    int m_iOffset;
    QFile m_qfMainCtrl;
};

#endif // QPOSTSVIEW_H
