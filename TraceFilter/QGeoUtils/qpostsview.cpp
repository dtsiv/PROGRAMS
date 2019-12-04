#include "qpostsview.h"
#include "tdcord.h"

#define RAD_2_DEG 57.29577951308232
#define DEG_2_RAD (.0174532925199432958)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPostsView::QPostsView(QString qsMainctrlCfg, QWidget *parent /* = 0 */)
    : QGraphicsView(parent) {
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
    PMAINCTRL m_pMainCtrl=NULL;
    if (qfMainCtrl.open(QIODevice::ReadOnly)) {
        m_pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
        qfMainCtrl.close();
        BLH blhViewPoint;
        if (TdCord::getViewPoint(&blhViewPoint,m_pMainCtrl)) { // radians,meters
            QList<XYZ> qlTcPosts;
            QList<int> qlPostIds;
            if (TdCord::getTopocentricPostsList(&blhViewPoint,qlTcPosts,qlPostIds,m_pMainCtrl)) {
                double dScale=1.0e-3;
                for (int i=0; i<qlTcPosts.count(); i++) {
                    XYZ postXYZ = qlTcPosts.at(i);
                    int iPostId = qlPostIds.at(i);
                    m_scene.addText(QString::number(iPostId))->setPos(postXYZ.dX*dScale,-postXYZ.dY*dScale);
                }
                return;
            }
        }
    }
    m_scene.addText("Open failed");
    return;
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
