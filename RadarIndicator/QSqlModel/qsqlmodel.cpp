#include "qsqlmodel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>


QSqlModel::QSqlModel() : QObject(0) {}

QSqlModel::~QSqlModel() {}

void QSqlModel::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QHBoxLayout *pHLayout=new QHBoxLayout;
    pPropPages->m_pleDBFileName=new QLineEdit("ChronicaParser.sqlite3");
    pHLayout->addWidget(new QLabel("DB file name"));
    pHLayout->addWidget(pPropPages->m_pleDBFileName);
    pWidget->setLayout(pHLayout);
    pTabWidget->insertTab(iIdx,pWidget,QSQLMODEL_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QSqlModel::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    m_qsDBFile = pPropPages->m_pleDBFileName->text();
    qDebug() << "m_qsDBFile = " << m_qsDBFile;
}


