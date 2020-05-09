#ifndef QNOISEMAPMGR_H
#define QNOISEMAPMGR_H

#include <QtGlobal>
#include <QObject>

#define     PARSE_PROGRESS_BAR_STEP         10
#define     PARSE_PROGRESS_BAR_MAX          100

class QIndicatorWindow;
class QSqlModel;
class QPoi;
class QNoiseMap;

class QNoiseMapMgr {
public:
    explicit QNoiseMapMgr(QIndicatorWindow *pOwner);
    void startGenerateNoiseMap(QObject *pSender);
    void updateGenerateNoiseMapProgressBar(bool bReset=false);

private:
    QIndicatorWindow *m_pOwner;
    QNoiseMap *m_pNoiseMap;
    QSqlModel *m_pSqlModel;
    QPoi *m_pPoi;
    int iNumberOfBeams;
    int iSizeOfComplex;
public:
    bool m_bStarted;
};

#endif // QNOISEMAPMGR_H
