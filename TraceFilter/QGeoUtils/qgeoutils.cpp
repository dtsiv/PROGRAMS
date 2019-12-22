#include "qgeoutils.h"
#include "qexceptiondialog.h"
#include "qinisettings.h"
#include "qpostsview.h"
#include "poitunit.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QGeoUtils::QGeoUtils()
        : m_qsMainctrlCfg(QString())
        , m_blhViewPoint({0,0,0}) /* BLH struct */
        , m_postBlh{{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} /* 9 array el-ts BLH */
        , m_postTc {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} /* 9 array el-ts XYZ */
         {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QGEOUTILS_MAINCTRL,"02.06.2017-09.00.22.639-mainctrl.cfg");
    m_qsMainctrlCfg = iniSettings.value(QGEOUTILS_MAINCTRL,scRes).toString();
    Legacy_TdCord::SetEllipsParams();
    if (!readMainCtrlCfg()) {
        qDebug() << "QGeoUtils::QGeoUtils(): readMainCtrlCfg() failed!";
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
    pGridLayout->addWidget(pPostsView,0,1,3,2,Qt::AlignVCenter | Qt::AlignLeft);
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseMainCtrl()));
    QObject::connect(pPropPages,SIGNAL(chooseMainCtrl()),SLOT(onMainctrlChoose()));
    pGridLayout->addWidget(new QLabel("View point geodetic coordinates"),3,0,1,3);
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
    pGridLayout->addWidget(new QLabel("Lat (deg)"),4,0);
    pGridLayout->addWidget(pPropPages->m_plbViewPtLat,4,1);
    pGridLayout->addWidget(new QLabel("Lon (deg)"),5,0);
    pGridLayout->addWidget(pPropPages->m_plbViewPtLon,5,1);
    pGridLayout->addWidget(new QLabel("Dummy"),0,3);
    pGridLayout->addWidget(new QLabel("DummyVal"),0,4);
    pGridLayout->addWidget(new QLabel("Dummy1"),1,3);
    pGridLayout->addWidget(new QLabel("Dummy1Val"),1,4);
    pGridLayout->addWidget(new QLabel("Dummy2"),2,3);
    pGridLayout->addWidget(new QLabel("Dummy2Val"),2,4);
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
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QString qsNewText = pPropPages->m_pleMainCtrl->text();
    if (qsNewText != m_qsMainctrlCfg) {
        m_qsMainctrlCfg = qsNewText;
        if (!readMainCtrlCfg()) {
            qDebug() << "QGeoUtils::propChanged(): readMainCtrlCfg() failed!";
        }
    }
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QGeoUtils::readMainCtrlCfg() {
    QFile qfMainCtrl(m_qsMainctrlCfg);
    PMAINCTRL pMainCtrl=NULL;
    if (qfMainCtrl.open(QIODevice::ReadOnly)) {
        pMainCtrl=(PMAINCTRL)qfMainCtrl.map(0,qfMainCtrl.size());
        qfMainCtrl.close();
        if (!pMainCtrl) return false;
        BLH blhViewPoint;
        if (TdCord::getViewPoint(&blhViewPoint,pMainCtrl)) { // radians,meters
            m_blhViewPoint = blhViewPoint;
            TdCord::GeodeticToTopocentricCache sCache;
            TdCord::initGeodeticToTopocentricCache(&m_blhViewPoint,&sCache);
            if (pMainCtrl->p.dwPosCount<=4 || pMainCtrl->p.dwPosCount>MAX_dwPosCount) return false;
            for (unsigned int i=1; i<=pMainCtrl->p.dwPosCount; i++) {
                PGROUNDINFO pgi = &pMainCtrl -> p.positions[i];
                m_postBlh[i].dLat=pgi->blh.dLat*DEG_TO_RAD;
                m_postBlh[i].dLon=pgi->blh.dLon*DEG_TO_RAD;
                m_postBlh[i].dHei=pgi->blh.dHei;
                TdCord::toTopocentric(&m_blhViewPoint,m_postBlh[i],&m_postTc[i],&sCache); // radians,meters
            }
        }
    }
    else {
        return false;
    }
    return true;
}
