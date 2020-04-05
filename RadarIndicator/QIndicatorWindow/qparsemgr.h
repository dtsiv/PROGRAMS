#ifndef QPARSEMGR_H
#define QPARSEMGR_H

#include <QtGlobal>
#include <QObject>

#define     PROGRESS_BAR_STEP         10
#define     PROGRESS_BAR_MAX          100

class QIndicatorWindow;
class QRegFileParser;
class QSqlModel;
class QTimer;

class QParseMgr {
public:
    explicit QParseMgr(QIndicatorWindow *pOwner);
    ~QParseMgr();
    void startParsing(QObject *pSender);
    void parseDataFile();
    void updateProgressBar(bool bReset=false);

private:
    QIndicatorWindow *m_pOwner;
    QRegFileParser *m_pRegFileParser;
    QSqlModel *m_pSqlModel;

};

#endif // QPARSEMGR_H
