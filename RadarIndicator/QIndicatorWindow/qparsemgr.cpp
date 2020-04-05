#include "qparsemgr.h"
#include "qindicatorwindow.h"
#include "qexceptiondialog.h"

double dProgressBarStep=(double)PROGRESS_BAR_STEP/PROGRESS_BAR_MAX;
int iNumberOfBeams = 4;
int iSizeOfComplex=2*sizeof(qint16);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QParseMgr::QParseMgr(QIndicatorWindow *pOwner) :
         m_pOwner(pOwner)
       , m_pRegFileParser(NULL) {
    // map members for convenience
    m_pRegFileParser           = m_pOwner->m_pRegFileParser;
    m_pSqlModel                = m_pOwner->m_pSqlModel;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QParseMgr::~QParseMgr() {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QParseMgr::startParsing(QObject *pSender) {

    // get access to QPropPages dynamic object (!take care - only exists while dlg)
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pSender);
    qDebug() << "startParsing(): stopping simulation timer";
    m_pOwner->m_simulationTimer.stop();
    m_pOwner->m_bParsingInProgress=true;
    pPropPages->m_ppbAccept->setEnabled(false);
    pPropPages->m_ppbParse->setEnabled(false);
    pPropPages->m_ppbarParseProgress->setMaximum(PROGRESS_BAR_MAX);

    // ensure progress bar updates
    QObject::connect(m_pOwner,SIGNAL(updateProgressBar(double)),pPropPages,SIGNAL(updateProgressBar(double)));
    QObject::connect(pPropPages,SIGNAL(updateProgressBar(double)),m_pOwner,SLOT(onUpdateProgressBar(double)));

    QString qsDbFileName = pPropPages->m_pleDBFileName->text();
    if (!m_pSqlModel->changeDatabaseName(qsDbFileName)) {
        throw RmoException(QString("Could not change DB to ")+qsDbFileName);
        return;
    }
    if (!m_pSqlModel->openDatabase()) {
        throw RmoException(QString("Could not open DB ")+qsDbFileName);
        return;
    }

    // start parsing registration file(s)
    bool bParseWholeDir = m_pRegFileParser->m_bParseWholeDir = pPropPages->m_pcbParseWholeDir->isChecked();

    // selected file
    QString qsRegFile = pPropPages->m_pleRegFileName->text();
    QFileInfo fiRegFile(qsRegFile);
    if (!fiRegFile.exists() || !fiRegFile.isFile()) {
        throw RmoException(QString("Could not find reg file ")+fiRegFile.absoluteFilePath());
        return;
    }
    QString qsCurFile = fiRegFile.absoluteFilePath();
    m_pRegFileParser->m_fiRegFile = fiRegFile;
    // extract time stamp
    QDateTime dtTimeStamp;
    QStringList qslDirs=qsCurFile.split("/");
    if (qslDirs.count()>1) {
        QString qsTimeStamp=qslDirs.at(qslDirs.count()-2);
        QStringList qslDateTime=qsTimeStamp.split("T");
        if (qslDateTime.count()==2) {
            dtTimeStamp=QDateTime(
                QDate::fromString(qslDateTime.at(0),"yyyy-MM-dd"),
                QTime::fromString(qslDateTime.at(1),"hh-mm-ss"),
                Qt::LocalTime);
        }
    }
    // by default use file time stamp
    m_pRegFileParser->m_uTimeStamp = fiRegFile.created().toMSecsSinceEpoch();
    // if possible use time stamp from file path
    if (dtTimeStamp.isValid()) {
        int iYear= dtTimeStamp.date().year();
        if (iYear > 1990 && iYear < 2100) {
            m_pRegFileParser->m_uTimeStamp = dtTimeStamp.toMSecsSinceEpoch();
        }
    }
    // parse single file and quit
    if (!bParseWholeDir) {
        if (m_pRegFileParser->openDataFile(qsCurFile)) {
            throw RmoException(QString("Could not open reg file ")+qsCurFile);
            return;
        }
        parseDataFile();
        m_pRegFileParser->closeDataFile();
        return;
    }
    // parse whole directory
    QDir qdRoot=fiRegFile.absoluteDir();
    QStringList qslFiles = qdRoot.entryList(QDir::Files);
    for (int j=0; j<qslFiles.count(); j++) {
        QRegExp rx("^\\d\\d\\d\\d\\d\\d\\d\\d$");
        // skip filenames those did not match pattern
        if (!rx.exactMatch(qslFiles.at(j))) continue;
        qsCurFile = qdRoot.absoluteFilePath(qslFiles.at(j));
        if (!qdRoot.exists(qsCurFile)) {
            throw RmoException(QString("Could not find reg file ")+qsCurFile);
            return;
        }
        if (m_pRegFileParser->openDataFile(qsCurFile)) {
            throw RmoException(QString("Could not open reg file ")+qsCurFile);
            return;
        }
        parseDataFile();
        m_pRegFileParser->closeDataFile();
    }
    pPropPages->m_ppbAccept->setEnabled(true);
    pPropPages->m_ppbParse->setEnabled(true);
    m_pOwner->m_bParsingInProgress=false;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QParseMgr::parseDataFile() {
    // qDebug() << "Parsing file " << m_pRegFileParser->m_qfRegFile.fileName();
    QString qsFilePath   = m_pRegFileParser->m_fiRegFile.absoluteFilePath();
    quint32 nRec         = m_pRegFileParser->m_sFileHdr.nRec;
    qint64 uTimeStamp    = m_pRegFileParser->m_uTimeStamp;
    quint32 uFileVersion = m_pRegFileParser->m_uFileVersion;
    qint64 iFileId=m_pSqlModel->addFileRec(uTimeStamp,qsFilePath,nRec,QString::number(uFileVersion,16));
    if (iFileId==-1) {
        throw RmoException(QString("addFileRec failed: %1 (nRec=%2, uFileVer=%3)").arg(qsFilePath).arg(nRec).arg(uFileVersion));
        return;
    }
    bool bReset=true;
    updateProgressBar(bReset);
    while(!m_pRegFileParser->isAtEnd()) {
        QByteArray *pbaStrobe = m_pRegFileParser->getStrobe();
        if (pbaStrobe==NULL || m_pRegFileParser->m_pDataHeader==NULL) {
            throw RmoException(QString("Could not get strob info from file ")+m_pRegFileParser->m_qfRegFile.fileName());
            return;
        }
        // most typical for now. Later - maybe more cases here
        if (m_pRegFileParser->m_pDataHeader->dwType == ACM_TYPE::STROBE_DATA && m_pRegFileParser->m_uFileVersion==REG::FORMAT_VERSION) {
            ACM::STROBE_DATA *pStrobeData = (ACM::STROBE_DATA *)pbaStrobe->data();
            CHR::STROBE_HEADER *pStrobeHeader = &pStrobeData->header;
            qint16* pSamples=(qint16*)(pStrobeData+1);
            quint32 uBeamCountsNum=pStrobeData->beamCountsNum;
            quint32 uStrobeNo=pStrobeHeader->strobeNo;
            QByteArray baStructStrobeData=QByteArray::fromRawData(pbaStrobe->data(),sizeof(ACM::STROBE_DATA));
            // start transaction for one strobe
            m_pSqlModel->startTransaction();
            qint64 iStrobId=m_pSqlModel->addStrobe(uStrobeNo,uBeamCountsNum,baStructStrobeData,iFileId);
            if (iStrobId==-1) {
                throw RmoException(QString("Could not add strobe record: StrobeNo=%1").arg(uStrobeNo));
                return;
            }
            for (int iBeam=0; iBeam<iNumberOfBeams; iBeam++) {
                if (int iRetVal=m_pSqlModel->addSamples(iStrobId,iBeam,(char*)pSamples,uBeamCountsNum*iSizeOfComplex)) {
                    throw RmoException(QString("AddSamples returned: %1").arg(iRetVal));
                    return;
                }
                pSamples+=uBeamCountsNum*2;
            }
            m_pSqlModel->commitTransaction();
            delete pbaStrobe;
        }
        updateProgressBar();
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QParseMgr::updateProgressBar(bool bReset /* =false */) {
    static int iCurr=0,iPrev=0;
    if (bReset) {
        iCurr=iPrev=0;
        emit m_pOwner->updateProgressBar(iCurr);
        return;
    }
    iCurr = qRound(m_pRegFileParser->getProgress()/dProgressBarStep);
    if (iCurr==iPrev) return;
    iPrev=iCurr;
    emit m_pOwner->updateProgressBar(iCurr*dProgressBarStep);
    int iMaxMSecs=500;
    QCoreApplication::processEvents(QEventLoop::AllEvents,iMaxMSecs);
}
