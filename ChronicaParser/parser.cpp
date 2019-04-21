#include "parser.h"

/* extern */ struct sFileHdr fileHdr;
struct sFileHdr *pFileHdr;

struct s_dataHeader{
    quint32 dwSize;
    quint32 dwType;
    char pFileData[];
};

int parseDataFile(quint64 uTimeStamp,QString qsFilePath, char *pFileData, quint64 uSize) {
    QTextStream tsStdOut(stdout);
    pFileHdr = (struct sFileHdr*)pFileData;
    fileHdr = *pFileHdr++;
    if (QString(QByteArray(fileHdr.sMagic,sizeof(fileHdr.sMagic)))!="REGI") return 1;
    if (fileHdr.uVer != 0x00000002) return 2;
    if (fileHdr.nRec > fileHdr.nRecMax || sizeof(struct sFileHdr)+fileHdr.nRec * sizeof(struct ACM_STROBE_DATA)>uSize) return 3;
    qint64 iFileId=addFileRec(uTimeStamp,qsFilePath,fileHdr.nRec);
    if (iFileId==-1) return 21;

	QSqlDatabase db=QSqlDatabase::database();
    if (!db.isOpen()) return 22;
    // run over all meaningful offsets
    quint32 *pOffsets=(quint32 *)(pFileData+sizeof(struct sFileHdr));
    // tsStdOut << "nRec = " << fileHdr.nRec << endl;
    for (quint32 i=0; i<fileHdr.nRec; i++) {

        quint64 uOffset=*pOffsets++;
        if (uOffset+sizeof(struct s_dataHeader)>uSize) return 4;
        s_dataHeader *pFileDataHdr = (s_dataHeader *)(pFileData+uOffset);
        // tsStdOut << i << "\tfound data hdr sz=" << pFileDataHdr->dwSize << " typ 0x" << QString::number(pFileDataHdr->dwType,16);
        if (pFileDataHdr->dwSize == 0) return 5;
        if (pFileDataHdr->dwType != ACM_TYPE_STROBE_DATA) continue;
        // dwType == ACM_TYPE_STROBE_DATA
        ACM_STROBE_DATA *pStrobeData=(ACM_STROBE_DATA *)&pFileDataHdr->pFileData[0];
        if (pStrobeData->beamCountsNum == 0) return 6;
        // tsStdOut << pStrobeData->strobeNo << "\t" << pStrobeData->beamCountsNum << endl;
        // ok, we have a valid strobe
        db.transaction();
        qint64 iStrobId=addStrobe(pStrobeData->strobeNo,pStrobeData->beamCountsNum,iFileId);
        if (iStrobId==-1) return 61;
        int iNumberOfBeams = 4;
        int iSizeOfComplex=2*sizeof(qint16);
        if (pFileDataHdr->dwSize != sizeof(ACM_STROBE_DATA)+iNumberOfBeams*pStrobeData->beamCountsNum*iSizeOfComplex
         || uOffset+sizeof(struct s_dataHeader)+pFileDataHdr->dwSize > uSize) return 7;
        // pointer to raw samples (Re,Im)
        qint16* pSamples=(qint16*)(pStrobeData+1);
        // loop over beams
        for (int iBeam=0; iBeam<4; iBeam++) {
            // tsStdOut << "\t" << iBeam;
            if (addSamples(iStrobId,iBeam,(char*)pSamples,pStrobeData->beamCountsNum*iSizeOfComplex)) return 8;
            pSamples+=pStrobeData->beamCountsNum*2;
        }
        db.commit();
        // tsStdOut << "\t commit" << endl;
    }
    return 0;
}
