#include "qpostsview.h"
#include "tdcord.h"

#define RAD_2_DEG 57.29577951308232
#define DEG_2_RAD (.0174532925199432958)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPostsView::QPostsView(QString qsMainctrlCfg, QWidget *parent /* = 0 */)
    : QGraphicsView(parent)
    , m_qsMainctrlCfg(QString())
    , m_blhViewPoint({0,0,0}) {
    setScene(&m_scene);
    scale(5,5);
    onMainctrlChanged(qsMainctrlCfg);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPostsView::onMainctrlChanged(QString qsMainctrlCfg) {
    m_scene.clear();
    QFile qfMainCtrl(qsMainctrlCfg);
    if (qfMainCtrl.open(QIODevice::ReadOnly)) {
        PMAINCTRL m_pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
        qfMainCtrl.close();
        double dScale=1.0e-3;
        getViewPoint(&m_blhViewPoint,m_pMainCtrl); // radians,meters
        for (int i=1; i<=4; i++) {
            PGROUNDINFO pgi = &m_pMainCtrl -> p.positions[i];
            XYZ postXYZ;
            BLH postBLH;
            postBLH.dLat=pgi->blh.dLat*DEG_2_RAD; postBLH.dLon=pgi->blh.dLon*DEG_2_RAD;
            postBLH.dHei=pgi->blh.dHei;
            TdCord::toTopocentric(&m_blhViewPoint,&postBLH,&postXYZ); // radians,meters
            m_scene.addText(QString::number(i))->setPos(postXYZ.dX*dScale,-postXYZ.dY*dScale);
        }
        m_qsMainctrlCfg = qsMainctrlCfg;
    }
    else {
        m_scene.addText("Open failed");
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPostsView::onMainctrlChoose() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("MainCtrl files (*.cfg)"));
    dialog.setDirectory(QDir::current());
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable()) {
            QDir qdSelFilePath=fiSelFilePath.absoluteDir();
            QDir qdCur=QDir::current();
            QString qsCompactPath;
            if (qdCur.rootPath()==qdSelFilePath.rootPath()) { // same Windows drive
                qsCompactPath=qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath());
            }
            else { // different Windows drives
                qsCompactPath=fiSelFilePath.absoluteFilePath();
            }
            onMainctrlChanged(qsCompactPath);
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//   pblhViewPoint - radians, meters
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPostsView::getViewPoint(PBLH pblhViewPoint, PMAINCTRL pMainCtrl) {
    pblhViewPoint->dLat=0.0e0; pblhViewPoint->dLon=0.0e0; pblhViewPoint->dHei=0.0e0;
    if (pMainCtrl->p.dwPosCount < 5) return;
    for (int i=1; i<=4; i++) {
        PGROUNDINFO pgi = &pMainCtrl -> p.positions[i];
        pblhViewPoint->dLat += pgi->blh.dLat*DEG_2_RAD;
        pblhViewPoint->dLon += pgi->blh.dLon*DEG_2_RAD;
    }
    pblhViewPoint->dLat/=4.0e0; pblhViewPoint->dLon/=4.0e0;
}
