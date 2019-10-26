#ifndef QSQLMODEL_H
#define QSQLMODEL_H

#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include <QMetaObject>

#include "QPropPages.h"

#define QSQLMODEL_PROP_TAB_CAPTION "DB connection"

class QSqlModel : public QObject {
	Q_OBJECT

public:
    QSqlModel();
	~QSqlModel();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

private:
	QString m_qsDBFile;
};


#endif // QSQLMODEL_H
