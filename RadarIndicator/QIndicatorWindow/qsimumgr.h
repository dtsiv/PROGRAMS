#ifndef QSIMUMGR_H
#define QSIMUMGR_H

#include <QtGlobal>
#include <QFile>
#include <QTextStream>

class QIndicatorWindow;
class QPoi;
class QSqlModel;
class QTargetsMap;

class QSimuMgr {
public:
    QSimuMgr(QIndicatorWindow *pOwner);
    ~QSimuMgr();
    void processStrob();

private:
    QIndicatorWindow *m_pOwner;
    QPoi *m_pPoi;
    QSqlModel *m_pSqlModel;
    QTargetsMap *m_pTargetsMap;
    QFile m_qfPeleng;
    QTextStream m_tsPeleng;
};

#endif // QSIMUMGR_H
