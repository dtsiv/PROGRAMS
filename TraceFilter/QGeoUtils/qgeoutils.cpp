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
    [[maybe_unused]]QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addWidget(new QPostsView(m_qsMainctrlCfg));
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QGEOUTILS_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QGeoUtils::propChanged(QObject *pPropDlg) {
    [[maybe_unused]]QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
}
