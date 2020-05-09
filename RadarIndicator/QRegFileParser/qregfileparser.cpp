#include "qregfileparser.h"
#include <windows.h>
#include <stdlib.h>

//====================================================================
//
//====================================================================
QRegFileParser::QRegFileParser(QString qsFilePath) : QObject(0)
         , m_uTimeStamp(0)
         , m_pFileHdr(NULL)
         , m_pStrobeData(NULL)
         , m_bParseWholeDir(false)
         , m_uFileVersion(0)
         , m_pOffsetsArray(NULL)
         , m_pOffsets(NULL)
         , m_uOffset(0)
         , m_uSize(0)
         , m_pDataHeader(0)
         , m_iRec(0) {

    m_pDataHeader = new struct s_dataHeader;

    // settings object and main window adjustments
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(SETTINGS_KEY_DATAFILE,QString("C:/PROGRAMS/DATA/RegFixed/2019-03-01T14-12-02/00000000"));
    m_qsRegFile = iniSettings.value(SETTINGS_KEY_DATAFILE,scRes).toString();
    iniSettings.setDefault(SETTINGS_KEY_PARSE_WHOLE_DIR,true);
    m_bParseWholeDir = iniSettings.value(SETTINGS_KEY_PARSE_WHOLE_DIR,scRes).toBool();
}
//====================================================================
//
//====================================================================
QRegFileParser::~QRegFileParser() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setValue(SETTINGS_KEY_DATAFILE, m_qsRegFile);
    iniSettings.setValue(SETTINGS_KEY_PARSE_WHOLE_DIR, m_bParseWholeDir);
    m_pOffsetsArray = NULL;
    m_pOffsets = NULL;
    m_pFileHdr = NULL;
    m_pStrobeData = NULL;
    if (m_pDataHeader) delete m_pDataHeader; m_pDataHeader = NULL;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRegFileParser::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QHBoxLayout *pHLayout=new QHBoxLayout;
    pPropPages->m_pleRegFileName=new QLineEdit(m_qsRegFile);
    pHLayout->addWidget(new QLabel("Reg file name"));
    pHLayout->addWidget(pPropPages->m_pleRegFileName);
    QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseRegFile()));
    QObject::connect(pPropPages,SIGNAL(chooseRegFile()),SLOT(onRegFileChoose()));
    pHLayout->addWidget(ppbChoose);
    QVBoxLayout *pVLayout=new QVBoxLayout;
    pVLayout->addLayout(pHLayout);
    QHBoxLayout *pHLayout01=new QHBoxLayout;
    pPropPages->m_pcbParseWholeDir=new QCheckBox("Parse whole dirrectory");
    pHLayout01->addWidget(pPropPages->m_pcbParseWholeDir);
    pPropPages->m_pcbParseWholeDir->setChecked(m_bParseWholeDir);
    pPropPages->m_ppbParse=new QPushButton("Parse");
    QObject::connect(pPropPages->m_ppbParse,SIGNAL(clicked(bool)),pPropPages,SIGNAL(doParse()));
    pHLayout01->addWidget(pPropPages->m_ppbParse);
    pPropPages->m_ppbarParseProgress=new QProgressBar;
    pPropPages->m_ppbarParseProgress->setValue(0);
    pHLayout01->addWidget(pPropPages->m_ppbarParseProgress);
    // pHLayout01->addStretch();
    pVLayout->addLayout(pHLayout01);
    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QREGFILEPARSER_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRegFileParser::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    m_bParseWholeDir = pPropPages->m_pcbParseWholeDir->isChecked();
    m_qsRegFile = pPropPages->m_pleRegFileName->text();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRegFileParser::onRegFileChoose() {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());

    QFileDialog dialog(pPropPages);
    QString qsRegFile = pPropPages->m_pleRegFileName->text();
    QFileInfo fiDataFile(qsRegFile);
    QDir qdRoot;
    if (!fiDataFile.exists() || !fiDataFile.isFile()) {
        qdRoot=QDir::current();
    }
    else {
        qdRoot=fiDataFile.absoluteDir();
    }
    dialog.setDirectory(qdRoot);
    dialog.setFileMode(QFileDialog::ExistingFile);
    // dialog.setNameFilter(tr("SQLite db file (*.db *.sqlite *.sqlite3)"));
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable())	{
            QDir qdCur = QDir::current();
            QDir qdSelFilePath = fiSelFilePath.absoluteDir();
            if (qdCur.rootPath() == qdSelFilePath.rootPath()) { // same Windows drives
                pPropPages->m_pleRegFileName->setText(qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath()));
            }
            else { // different Windows drives
                pPropPages->m_pleRegFileName->setText(fiSelFilePath.absoluteFilePath());
            }
        }
        else {
            pPropPages->m_pleRegFileName->setText("error");
        }
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int QRegFileParser::openDataFile(QString qsRegFile) {

    QFile qfRegFile(qsRegFile);
    if (!qfRegFile.exists() || !qfRegFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file: " << QFileInfo(qfRegFile).absoluteFilePath();
        return 1;
    }
    qfRegFile.close();
    m_qfRegFile.setFileName(qsRegFile);
    if (!m_qfRegFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file: " << QFileInfo(m_qfRegFile).absoluteFilePath();
        return 2;
    }

    if (m_qfRegFile.read((char*)&m_sFileHdr,sizeof(m_sFileHdr)) != sizeof(m_sFileHdr)) return 3;
    if (QString(QByteArray(m_sFileHdr.sMagic,sizeof(m_sFileHdr.sMagic)))!=REG::FILE_MAGIC) return 4;

    // version of the registration file
    if (  m_sFileHdr.uVer != REG_OLD_VER1::FORMAT_VERSION
       && m_sFileHdr.uVer != REG::FORMAT_VERSION) return 5;
    m_uFileVersion = m_sFileHdr.uVer;
    m_uSize = m_qfRegFile.size();
    if (m_uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
        if (m_sFileHdr.nRec > m_sFileHdr.nRecMax
         || sizeof(m_sFileHdr)+m_sFileHdr.nRecMax*sizeof(quint32)+m_sFileHdr.nRec*sizeof(struct ACM_OLD_VER1::ACM_STROBE_DATA) > m_uSize) return 6;
    }
    if (m_uFileVersion==REG::FORMAT_VERSION) {
        if (m_sFileHdr.nRec > m_sFileHdr.nRecMax
         || sizeof(struct sFileHdr)+sizeof(quint32)+m_sFileHdr.nRecMax*sizeof(quint32)+m_sFileHdr.nRec*sizeof(struct ACM::STROBE_DATA) > m_uSize) return 7;
    }
    // DTSIV: наверное, uProtoVersion не используется и может привести к ошибке потом
    if (m_uFileVersion==REG::FORMAT_VERSION) {
        quint32 uProtoVersion=0;
        if (m_qfRegFile.read((char*)&uProtoVersion,sizeof(quint32)) != sizeof(quint32)) return 1;
        // tsStdOut << "uProtoVersion = " << uProtoVersion << endl;
    }
    m_pOffsetsArray=new quint32[m_sFileHdr.nRecMax];
    m_pOffsets=m_pOffsetsArray;
    if (m_qfRegFile.read((char*)m_pOffsets,m_sFileHdr.nRecMax*sizeof(quint32)) != m_sFileHdr.nRecMax*sizeof(quint32)) return 8;
    m_uOffset=0;
    if (m_uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
        m_uOffset = *m_pOffsets++;
    }
    else if (m_uFileVersion==REG::FORMAT_VERSION) {
        m_uOffset = m_qfRegFile.pos();
    }
    else {
        return 9;
    }
    m_iRec=0;
    return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int QRegFileParser::closeDataFile() {
    m_uFileVersion=0;
    m_uSize=0;
    if (m_pOffsetsArray) delete[]m_pOffsetsArray;
    m_pOffsetsArray=NULL;
    m_pOffsets=NULL;
    m_uOffset=0;
    std::memset((char*)&m_sFileHdr,'\0',sizeof(m_sFileHdr));
    m_iRec=0;
    if (!m_qfRegFile.isOpen()) {
        qDebug() << "file not open: " << QFileInfo(m_qfRegFile).absoluteFilePath();
        return 1;
    }
    m_qfRegFile.close();
    return 0;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool QRegFileParser::isAtEnd() {
    if (m_qfRegFile.isOpen() && m_iRec < m_sFileHdr.nRec && m_uOffset+sizeof(struct s_dataHeader) < m_uSize) return false;
    return true;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double QRegFileParser::getProgress() {
    if (m_sFileHdr.nRec < 1 || m_iRec>m_sFileHdr.nRec) return 0.0;
    return (double)m_iRec/m_sFileHdr.nRec;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QByteArray * QRegFileParser::getStrobe() {
    // qDebug() << "iRec=" << iRec << " m_sFileHdr.nRec=" << m_sFileHdr.nRec;
    if (m_iRec>m_sFileHdr.nRec) return NULL;
    m_uOffset = *m_pOffsets++;
    // qDebug() << "m_uOffset+sizeof(struct s_dataHeader)=" << m_uOffset+sizeof(struct s_dataHeader) << " m_uSize=" << m_uSize;
    if (m_uOffset+sizeof(struct s_dataHeader) > m_uSize) return NULL;
    // qDebug() << "m_uOffset=" << m_uOffset;
    m_qfRegFile.seek(m_uOffset);
    quint32 uDataHeaderSize = m_qfRegFile.read((char*)m_pDataHeader,sizeof(struct s_dataHeader));
    // qDebug() << "uDataHeaderSize=" << uDataHeaderSize << " sizeof(struct s_dataHeader)=" << sizeof(struct s_dataHeader);
    if (uDataHeaderSize != sizeof(struct s_dataHeader)) return NULL;
    // qDebug() << "m_pDataHeader->dwSize=" << m_pDataHeader->dwSize;
    // qDebug() << "m_uOffset + sizeof(struct s_dataHeader) + m_pDataHeader->dwSize=" << m_uOffset + sizeof(struct s_dataHeader) + m_pDataHeader->dwSize;
    // qDebug() << "m_uSize=" << m_uSize;
    if ((m_pDataHeader->dwSize == 0) || (m_uOffset + sizeof(struct s_dataHeader) + m_pDataHeader->dwSize > m_uSize)) return NULL;
    QByteArray *pbaRetVal = new QByteArray(m_pDataHeader->dwSize,'\0');
    // qDebug() << "m_pDataHeader->dwSize=" << m_pDataHeader->dwSize << " pbaRetVal=" <<pbaRetVal;
    if (!pbaRetVal) return NULL;
    quint32 uStrobSize = m_qfRegFile.read((char*)pbaRetVal->data(),m_pDataHeader->dwSize);
    // qDebug() << "uStrobSize=" << uStrobSize << " m_pDataHeader->dwSize=" << m_pDataHeader->dwSize;
    if (uStrobSize != m_pDataHeader->dwSize) {
        delete pbaRetVal; return NULL;
    }
    m_iRec++;
    return pbaRetVal;
}

#if 0
{
    QFileInfo fiDataFile(m_qsRegFile);
    if (!fiDataFile.exists() || !fiDataFile.isFile()) return 1;
    QDir qdRoot=fiDataFile.absoluteDir();
    // tsStdOut << "browsing: " << qdRoot.absolutePath() << endl;

    QStringList qslFiles = qdRoot.entryList(QDir::Files);
    for (int j=0; j<qslFiles.count(); j++) {
        QRegExp rx("^\\d\\d\\d\\d\\d\\d\\d\\d$");
        if (!rx.exactMatch(qslFiles.at(j))) continue;
        QString qsCurFile=qdRoot.absoluteFilePath(qslFiles.at(j));
        if (!qdRoot.exists(qsCurFile)) return 4;
        // we only peek one file
        if (qsCurFile!=fiDataFile.absoluteFilePath()) continue;
        tsStdOut << "opening: " << qsCurFile << endl;
        // open
        QFile qfCurFile(qsCurFile);
        if (!qfCurFile.exists() || !qfCurFile.open(QIODevice::ReadOnly)) {
            tsStdOut << "\nCannot open file: " << QFileInfo(qfCurFile).absoluteFilePath()  << "\n\n";
            return 5;
        }

        // parse file name
        // tsStdOut << "Parsing" << endl;
        QStringList qslDirs=qsCurFile.split("/");
        if (qslDirs.count()<2) continue;
        QString qsTimeStamp=qslDirs.at(qslDirs.count()-2);
        QStringList qslDateTime=qsTimeStamp.split("T");
        // tsStdOut << "qslDateTime" << endl;
        if (qslDateTime.count()!=2) continue;
        dtTimeStamp=QDateTime(
            QDate::fromString(qslDateTime.at(0),"yyyy-MM-dd"),
            QTime::fromString(qslDateTime.at(1),"hh-mm-ss"),
            Qt::LocalTime);
        if (!dtTimeStamp.isValid()) continue;

        // map & parse contents

        iRetval=parseDataFile(dtTimeStamp.toMSecsSinceEpoch(),qsCurFile, &qfCurFile, qfCurFile.size());
        if (iRetval) {
            tsStdOut << "\nparseDataFile() returned: " << iRetval << endl;
            return 100+iRetval;
        }
        tsStdOut << "parsed: " << qsTimeStamp << endl;

        qfCurFile.close();
        // !!! For debug:
        // !!! just parse first file in target dir
        // break;
    }
    return 0;
}
#endif
