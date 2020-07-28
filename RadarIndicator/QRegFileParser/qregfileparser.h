#ifndef QREGFILEPARSER_H
#define QREGFILEPARSER_H

#include <QtCore>
#include <QMetaObject>

#include "qproppages.h"
#include "qinisettings.h"

#include "qchrprotosnc.h"
#include "qchrprotoacm.h"

#define QREGFILEPARSER_PROP_TAB_CAPTION "Parse registration file"

#define SETTINGS_KEY_DATAFILE           "dataFile"
#define SETTINGS_KEY_PARSE_WHOLE_DIR    "parseWholeDir"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200)
#endif


class QRegFileParser : public QObject {
    Q_OBJECT

    struct sFileHdr {
        char sMagic[4];
        quint32 uVer;
        quint32 nRecMax;
        quint32 nRec;
    };

    struct s_dataHeader{
        quint32 dwSize;
        quint32 dwType;
        char pFileData[];
    };

public:
    QRegFileParser(QString qsFilePath);
    ~QRegFileParser();
    int openDataFile(QString qsRegFile);
    int closeDataFile();
    QByteArray *getStrobe();
    bool isAtEnd();
    double getProgress();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

public slots:
    void onRegFileChoose();

public:
    QString              m_qsRegFile;
    quint64              m_uTimeStamp;
    QFile                m_qfRegFile;
    QFileInfo            m_fiRegFile;
private:
    struct sFileHdr     *m_pFileHdr;
    ACM::STROBE_DATA    *m_pStrobeData;
public:
    bool                 m_bParseWholeDir;
    struct sFileHdr      m_sFileHdr;
    quint32              m_uFileVersion;
private:
    quint32              *m_pOffsetsArray;
    quint32              *m_pOffsets;
    quint32              m_uOffset;
    quint32              m_uSize;
public:
    struct s_dataHeader  *m_pDataHeader;
private:
    quint32              m_iRec;
};

//extern int iDopplerFrom,iDopplerTo,iDelayFrom,iDelayTo;
//extern double dThreshold,dRelThresh;
//extern int nFalseAlarms;
//extern double dCarrierF, dTs;
//extern int NT,NT_,Np,Ntau;
//extern int iPlotSlicePeriod,iPlotSliceBeam;

//extern struct sFileHdr fileHdr;

//int readSettings();
//int openDataFile();
//int createTables();
//int dropTables();
//int openDatabase();
//int closeDatabase();
//qint64 addFileRec(quint64 uTimeStamp, QString qsFilePath, int nStrobs, QString qsFileVer);
//qint64 addStrobe(int strobeNo, int beamCountsNum, QByteArray baStrobeHeader, qint64 iFileId);
//int addSamples(qint64 iStrobId, int iBeam, char *pSamples, int iSize);


//unsigned int getFileVersion();
//unsigned int getProtoVersion();

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // QREGFILEPARSER_H


