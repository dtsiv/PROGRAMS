#include "qgeoutils.h"
#include "qexceptiondialog.h"
#include "qinisettings.h"
#include "qpostsview.h"
#include "poitunit.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::QGeoUtils()
        : m_qsMainctrlCfg(QString()) {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QGEOUTILS_MAINCTRL,"02.06.2017-09.00.22.639-mainctrl.cfg");
    m_qsMainctrlCfg = iniSettings.value(QGEOUTILS_MAINCTRL,scRes).toString();
    Legacy_TdCord::SetEllipsParams();

    QFile qfMainCtrl(m_qsMainctrlCfg);
    PMAINCTRL pMainCtrl=NULL;
    if (qfMainCtrl.open(QIODevice::ReadOnly)) {
        pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
        qfMainCtrl.close();
        BLH blhViewPoint;
        if (TdCord::getViewPoint(&blhViewPoint,pMainCtrl)) { // radians,meters
            // qDebug() << "ViewPoint: " <<  "(" << blhViewPoint.dLat <<  ", " << blhViewPoint.dLon <<  ", " << blhViewPoint.dHei <<  "); ";
            m_blhViewPoint = blhViewPoint;
            // TEST
            // TEST
            // TEST
            // TdCord::GeodeticToTopocentricCache sCache;
            // TdCord::initGeodeticToTopocentricCache(&blhViewPoint,&sCache);
            // QList<XYZ> qlTcPosts;
            // QList<int> qlPostIds;
            // if (TdCord::getTopocentricPostsList(&blhViewPoint,qlTcPosts,qlPostIds,pMainCtrl)) {
            //
                // for (int i=0; i<qlTcPosts.count(); i++) {
                //     XYZ postXYZ = qlTcPosts.at(i);
                //     BLH blhPostBL;
                //     int iPostId = qlPostIds.at(i);
                //     TdCord::fromTopocentric(&blhViewPoint,postXYZ,&blhPostBL,&sCache);
                //     BLH blhPos = pMainCtrl->p.positions[iPostId].blh;
                //     blhPos.dLat*=DEG_TO_RAD;
                //     blhPos.dLon*=DEG_TO_RAD;
                //     // qDebug() << "Post " << iPostId << ": "
                //     //          <<  "(" << blhPostBL.dLat <<  ", " << blhPostBL.dLon <<  ", " << blhPostBL.dHei <<  "); ";
                //     // qDebug() << "Topocentric " <<  "(" << postXYZ.dX <<  ", " << postXYZ.dY <<  ", " << postXYZ.dZ <<  "); ";
                //     // qDebug() << "Original: "
                //     //          <<  "(" << blhPos.dLat <<  ", " << blhPos.dLon <<  ", " << blhPos.dHei <<  "); " ;
                // }
            // }
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::~QGeoUtils() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(QGEOUTILS_MAINCTRL, m_qsMainctrlCfg);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QVBoxLayout *pVLayout = new QVBoxLayout;
    QHBoxLayout *pHLayout = new QHBoxLayout;
    pHLayout->addWidget(new QLabel("cfg file:"));
    pPropPages->m_pleMainCtrl=new QLineEdit(m_qsMainctrlCfg);
    pPropPages->m_pleMainCtrl->setMaximumWidth(QWIDGETSIZE_MAX);
    pPropPages->m_pleMainCtrl->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);
    pHLayout->addWidget(pPropPages->m_pleMainCtrl);
    pHLayout->setStretch(1,100);
    QPushButton *ppbChoose = new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
    pHLayout->addWidget(ppbChoose);
    QGridLayout *pGridLayout=new QGridLayout;
    pGridLayout->addWidget(new QLabel("Posts map:"),0,0);
    QPostsView *pPostsView = new QPostsView(m_qsMainctrlCfg);
    pPropPages->m_pPostsView=pPostsView;
    pGridLayout->addWidget(pPostsView,1,0,3,3);
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseMainCtrl()));
    QObject::connect(pPropPages,SIGNAL(chooseMainCtrl()),SLOT(onMainctrlChoose()));
    pGridLayout->addWidget(new QLabel("View point geodetic coordinates"),4,0,1,3);
    double dVPLat=0, dVPLon=0;
    if (pPostsView) {
        BLH blhViewPoint;
        if (TdCord::getViewPoint(&blhViewPoint,pPostsView->m_pMainCtrl)) { // radians,meters
            // qDebug() << "ViewPoint: " <<  "(" << blhViewPoint.dLat <<  ", " << blhViewPoint.dLon <<  ", " << blhViewPoint.dHei <<  "); ";
            m_blhViewPoint = blhViewPoint;
            dVPLat = m_blhViewPoint.dLat * RAD_TO_DEG;
            dVPLon = m_blhViewPoint.dLon * RAD_TO_DEG;
        }
    }
    pPropPages->m_plbViewPtLat = new QLabel(QString::number(dVPLat,'f',6));
    pPropPages->m_plbViewPtLon = new QLabel(QString::number(dVPLon,'f',6));
    pGridLayout->addWidget(new QLabel("Lat (deg)"),5,0);
    pGridLayout->addWidget(pPropPages->m_plbViewPtLat,5,1);
    pGridLayout->addWidget(new QLabel("Lon (deg)"),6,0);
    pGridLayout->addWidget(pPropPages->m_plbViewPtLon,6,1);
    pGridLayout->setColumnStretch(2,100);
    pGridLayout->setColumnStretch(5,100);
    pVLayout->addLayout(pHLayout);
    pVLayout->addLayout(pGridLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QGEOUTILS_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::propChanged(QObject *pPropDlg) {
    [[maybe_unused]]QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);

//    QFile qfMainCtrl(qsMainctrlCfg);
//    PMAINCTRL pMainCtrl=NULL;
//    if (qfMainCtrl.open(QIODevice::ReadOnly)) {
//        pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
//        qfMainCtrl.close();
//        BLH blhViewPoint;
//        if (TdCord::getViewPoint(&blhViewPoint,pMainCtrl)) { // radians,meters
//            QList<XYZ> qlTcPosts;
//            QList<int> qlPostIds;
//            if (TdCord::getTopocentricPostsList(&blhViewPoint,qlTcPosts,qlPostIds,pMainCtrl)) {
//                double dScale=1.0e-3;
//                for (int i=0; i<qlTcPosts.count(); i++) {
//                    XYZ postXYZ = qlTcPosts.at(i);
//                    int iPostId = qlPostIds.at(i);
//                    m_scene.addText(QString::number(iPostId))->setPos(postXYZ.dX*dScale,-postXYZ.dY*dScale);
//                }
//                return;
//            }
//        }
//    }
//    m_scene.addText("Open failed");
//    return;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::onMainctrlChoose() {
    // Show File Dialog
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());

    QString qsOldText = pPropPages->m_pleMainCtrl->text();
    QFileDialog dialog(pPropPages);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("MainCtrl files (*.cfg)"));
    dialog.setDirectory(QDir::current());
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable()) {
            QDir qdCur=QDir::current();
            QDir qdSelFilePath=fiSelFilePath.absoluteDir();
            if (qdCur.rootPath()==qdSelFilePath.rootPath()) { // same Windows drive
                pPropPages->m_pleMainCtrl->setText(qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath()));
            }
            else { // different Windows drives
                pPropPages->m_pleMainCtrl->setText(fiSelFilePath.absoluteFilePath());
            }
        }
        else {
            pPropPages->m_pleMainCtrl->setText("error");
        }
    }
    // no change - no job!!
    if (qsOldText == pPropPages->m_pleMainCtrl->text()) return;

    // Update View Point
    double dVPLat=0,dVPLon=0;
    QPostsView *pPostsView = qobject_cast<QPostsView *> (pPropPages->m_pPostsView);
    if (pPostsView) {
        if (pPostsView->onMainctrlChanged(pPropPages->m_pleMainCtrl->text())) {
            dVPLat = pPostsView->m_blhViewPont.dLat * RAD_TO_DEG;
            dVPLon = pPostsView->m_blhViewPont.dLon * RAD_TO_DEG;
        }
    }
    if (pPropPages->m_plbViewPtLat && pPropPages->m_plbViewPtLon) {
        pPropPages->m_plbViewPtLat->setText(QString::number(dVPLat,'f',6));
        pPropPages->m_plbViewPtLon->setText(QString::number(dVPLon,'f',6));
    }
}
