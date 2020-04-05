#ifndef QSIMUMGR_H
#define QSIMUMGR_H

#include <QtGlobal>
#include <QFile>

class QIndicatorWindow;

class QSimuMgr {
public:
    QSimuMgr(QIndicatorWindow *pOwner);
    ~QSimuMgr();
    void processStrob();

private:
    QIndicatorWindow *m_pOwner;
    QFile m_qfPeleng;
};

#endif // QSIMUMGR_H
