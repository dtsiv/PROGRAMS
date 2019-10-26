#include "qsqlmodel.h"
#include "qinisettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

QSqlModel::QSqlModel() : QObject(0)
        , m_qsDBFile("Uninitialized") {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(SETTINGS_SQLITE_FILE,"ChronicaParser.sqlite3");
    m_qsDBFile = iniSettings.value(SETTINGS_SQLITE_FILE,scRes).toString();
}

QSqlModel::~QSqlModel() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(SETTINGS_SQLITE_FILE, m_qsDBFile);
}

void QSqlModel::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QHBoxLayout *pHLayout=new QHBoxLayout;
    pPropPages->m_pleDBFileName=new QLineEdit(m_qsDBFile);
    pHLayout->addWidget(new QLabel("DB file name"));
    pHLayout->addWidget(pPropPages->m_pleDBFileName);
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pHLayout);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QSQLMODEL_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    m_qsDBFile = pPropPages->m_pleDBFileName->text();
    // qDebug() << "m_qsDBFile = " << m_qsDBFile;
}


