#ifndef QPARSEMGR_H
#define QPARSEMGR_H

#include <QtGlobal>
#include <QObject>

#define     PARSE_PROGRESS_BAR_STEP         10
#define     PARSE_PROGRESS_BAR_MAX          100

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
    void updateParseProgressBar(bool bReset=false);

private:
    QIndicatorWindow *m_pOwner;
    QRegFileParser *m_pRegFileParser;
    QSqlModel *m_pSqlModel;
    int iNumberOfBeams;
    int iSizeOfComplex;
};

#endif // QPARSEMGR_H
