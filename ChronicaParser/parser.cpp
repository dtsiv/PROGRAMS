#include "parser.h"

/* extern */ struct sFileHdr fileHdr;

struct s_dataHeader{
    quint32 dwSize;
    quint32 dwType;
    char pFileData[];
};

quint32 uFileVersion=0;
quint32 uProtoVersion=0;

int parseDataFile(quint64 uTimeStamp, QString qsFilePath, QFile *qfCurFile, quint64 uSize) {
    QTextStream tsStdOut(stdout);
    if (!qfCurFile->isOpen()) return 1;

    // check magic string "REGI"
    if (qfCurFile->read((char*)&fileHdr,sizeof(struct sFileHdr)) != sizeof(struct sFileHdr)) return 1;
    if (QString(QByteArray(fileHdr.sMagic,sizeof(fileHdr.sMagic)))!=REG::FILE_MAGIC) return 1;

    // version of the registration file
    uFileVersion = fileHdr.uVer;
    qint64 iFileId=addFileRec(uTimeStamp,qsFilePath,fileHdr.nRec,QString::number(uFileVersion,16));
    if (iFileId==-1) return 21;

    // tsStdOut << "uFileVersion = " << uFileVersion << endl;
    if (  fileHdr.uVer != REG_OLD_VER1::FORMAT_VERSION
       && fileHdr.uVer != REG::FORMAT_VERSION) return 2;

    if (uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
        if (fileHdr.nRec > fileHdr.nRecMax
         || sizeof(struct sFileHdr)+fileHdr.nRecMax*sizeof(quint32)+fileHdr.nRec*sizeof(struct ACM_OLD_VER1::ACM_STROBE_DATA) > uSize) return 3;
    }
    if (uFileVersion==REG::FORMAT_VERSION) {
        if (fileHdr.nRec > fileHdr.nRecMax
         || sizeof(struct sFileHdr)+sizeof(quint32)+fileHdr.nRecMax*sizeof(quint32)+fileHdr.nRec*sizeof(struct ACM::STROBE_DATA) > uSize) return 3;
    }

    // DTSIV: наверное, uProtoVersion не используется и может привести к ошибке потом
    if (uFileVersion==REG::FORMAT_VERSION) {
        if (qfCurFile->read((char*)&uProtoVersion,sizeof(quint32)) != sizeof(quint32)) return 1;
        // tsStdOut << "uProtoVersion = " << uProtoVersion << endl;
    }

    quint32 *pOffsetsArray=new quint32[fileHdr.nRecMax];
    quint32 *pOffsets=pOffsetsArray;
    if (qfCurFile->read((char*)pOffsets,fileHdr.nRecMax*sizeof(quint32)) != fileHdr.nRecMax*sizeof(quint32)) return 1;
    quint32 uOffset=0;
    if (uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
        uOffset = *pOffsets++;
    }
    else if (uFileVersion==REG::FORMAT_VERSION) {
        uOffset=qfCurFile->pos();
    }
    else {
        return 2;
    }

    // tsStdOut << "nRec = " << fileHdr.nRec << endl;
    // tsStdOut << "nRecMax = " << fileHdr.nRecMax << endl;

    QSqlDatabase db=QSqlDatabase::database();
    if (!db.isOpen()) return 22;

    // Loop over all records within current file
    for (quint32 i=0; i<fileHdr.nRec; i++) {
        // !!! for debug:
        // !!! just parse 4 first records whatever they are
        // if (i>3) break;

        // seek next s_dataHeader and validate it
        if (uOffset+sizeof(struct s_dataHeader)>uSize) return 4;
        qfCurFile->seek(uOffset);
        s_dataHeader *pFileDataHdr = new struct s_dataHeader;
        if (qfCurFile->read((char*)pFileDataHdr,sizeof (struct s_dataHeader)) != sizeof (struct s_dataHeader)) return 1;
        // tsStdOut << i << "\t" << QString::number(uOffset,16) << "\t" << pFileDataHdr->dwSize << "\t 0x" << QString::number(pFileDataHdr->dwType,16) << endl;
        if (pFileDataHdr->dwSize == 0) return 5;

        // read data s_dataHeader::pFileData[]
        char *pData=new char[pFileDataHdr->dwSize];
        if (qfCurFile->read((char*)pData,pFileDataHdr->dwSize) != pFileDataHdr->dwSize) return 1;

        //############# record header type SNC_TYPE::STROBE (radar probing settimgs)######################
        if (pFileDataHdr->dwType == SNC_TYPE::STROBE) {
            SNC_OLD_VER1::SNC_STROBE* pOldVer1Strobe=0;
            SNC::STROBE* pStrobe=0;
            CHR::STROBE_HEADER strobeHeader;

            // old format version
            if (uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
                pOldVer1Strobe = (SNC_OLD_VER1::SNC_STROBE*)pData;
                // tsStdOut << "found old SNC_OLD_VER1::SNC_STROBE" << endl;
                tsStdOut << "SNC_OLD_VER1::SNC_STROBE(0x" << QString::number(pFileDataHdr->dwType,16) << ")" << "\t"
                         << QString::number(pOldVer1Strobe->execTime*1.0e0,'g',8).rightJustified(10) << "\t"
                         << pOldVer1Strobe->azimuth << "\t"
                         << pOldVer1Strobe->elevation << "\t"
                         << pOldVer1Strobe->strobeNo << "\t"
                         << pOldVer1Strobe->flags << "\t"
                         << pOldVer1Strobe->pulsesCount << "\t"
                         << endl;
            }
            // current format version
            else if (uFileVersion==REG::FORMAT_VERSION) {
                pStrobe = (SNC::STROBE*)pData;
                // tsStdOut << "found new SNC::STROBE" << endl;
                strobeHeader=pStrobe->header;
                tsStdOut << "SNC::STROBE(0x" << QString::number(pFileDataHdr->dwType,16) << ")" << "\t"
                         << QString::number(strobeHeader.execTime*1.0e0,'g',8).rightJustified(10) << "\t"
                         << pStrobe->azimuth << "\t"
                         << pStrobe->elevation << "\t"
                         << strobeHeader.strobeNo << "\t"
                         << strobeHeader.flags << "\t"
                         << strobeHeader.pCount << "\t"
                         << strobeHeader.distance << "\t"
                         << strobeHeader.pPeriod << "\t"
                         << endl;
            }
            // unknown format version
            else {
                tsStdOut << "pFileDataHdr->dwType = 0x" << QString::number(pFileDataHdr->dwType,16)
                         << " uFileVersion=" << uFileVersion << endl;
                return 11;
            }

        }
        //############# record header type STROBE_DATA (beamCountsNum*4beams complex 16bit pairs)######################
        else if (pFileDataHdr->dwType == ACM_TYPE::STROBE_DATA) {
            qint16* pSamples;
            quint32 uStrobeNo;
            quint32 uBeamCountsNum;
            int iNumberOfBeams = 4;
            int iSizeOfComplex=2*sizeof(qint16);
            CHR::STROBE_HEADER strobeHeader;

            // old format version
            if (uFileVersion==REG_OLD_VER1::FORMAT_VERSION) {
                // dwType == ACM_TYPE::STROBE_DATA
                if (pFileDataHdr->dwType != ACM_OLD_VER1::ACM_TYPE_STROBE_DATA) return 13;
                ACM_OLD_VER1::ACM_STROBE_DATA *pStrobeData=(ACM_OLD_VER1::ACM_STROBE_DATA *)pData;
                if (pStrobeData->beamCountsNum == 0) return 6;
                // tsStdOut << "found old ACM_OLD_VER1::ACM_STROBE_DATA" << endl;
                tsStdOut << "ACM_OLD_VER1::ACM_STROBE_DATA(0x" << QString::number(pFileDataHdr->dwType,16) << ")" << "\t"
                         << QString::number(pStrobeData->execTime*1.0e0,'g',8).rightJustified(10) << "\t"
                         << pStrobeData->inclEpsilon << "\t"
                         << pStrobeData->inclBeta << "\t"
                         << pStrobeData->strobeNo << "\t"
                         << pStrobeData->flags << "\t"
                         << pStrobeData->beamCountsNum << "\t"
                         << endl;
                // tsStdOut << pStrobeData->strobeNo << "\t" << pStrobeData->beamCountsNum << endl;
                // ok, we have a valid strobe
                uStrobeNo=pStrobeData->strobeNo;
                uBeamCountsNum=pStrobeData->beamCountsNum;
                pSamples=(qint16*)(pStrobeData+1);
                // check uBeamCountsNum against file size
                if (pFileDataHdr->dwSize != sizeof(ACM_OLD_VER1::ACM_STROBE_DATA)+iNumberOfBeams*uBeamCountsNum*iSizeOfComplex
                 || uOffset+sizeof(struct s_dataHeader)+pFileDataHdr->dwSize > uSize) return 7;
            }
            // current format version
            else if (uFileVersion==REG::FORMAT_VERSION) {
                ACM::STROBE_DATA *pStrobeData=(ACM::STROBE_DATA *)pData;
                if (pStrobeData->beamCountsNum == 0) return 6;
                // tsStdOut << pStrobeData->strobeNo << "\t" << pStrobeData->beamCountsNum << endl;
                // ok, we have a valid strobe
                CHR::STROBE_HEADER *pStrobeHeader = &pStrobeData->header;
                tsStdOut << "\t" << " ACM::STROBE_DATA(0x" << QString::number(pFileDataHdr->dwType,16) << ")" << "\t"
                         << QString::number(pStrobeHeader->execTime).rightJustified(10) << "\t"
                         << pStrobeData->inclEpsilon << "\t"
                         << pStrobeData->inclBeta << "\t"
                         << pStrobeHeader->strobeNo << "\t"
                         << pStrobeHeader->flags << "\t"
                         << pStrobeData->beamCountsNum << "\t"
                         << pStrobeHeader->distance << "\t"
                         << pStrobeHeader->blank << "\t"
                         << pStrobeHeader->signalID << "\t"
                         << pStrobeHeader->pPeriod << "\t"
                         << pStrobeHeader->pCount << "\t"
                         << pStrobeHeader->pDuration << "\t"
                         << pStrobeHeader->padding << "\t"
                         << pStrobeData->velocity << "\t"
                         << pStrobeData->timeParams << "\t"
                         << endl;
                strobeHeader=*pStrobeHeader;
                uStrobeNo=pStrobeHeader->strobeNo;
                uBeamCountsNum=pStrobeData->beamCountsNum;
                pSamples=(qint16*)(pStrobeData+1);

                if (uStrobeNo >21643) {
                    if (pStrobeHeader->distance == 0 || pStrobeHeader->pPeriod!=200) {
                        // uOffset=*pOffsets++;
                        if (pFileDataHdr) delete pFileDataHdr; pFileDataHdr=0;
                        if (pData) delete []pData; pData=0;
                        break;
                    }
                }
                //else {
                //    tsStdOut << "nRec = " << fileHdr.nRec << endl;
                //    tsStdOut << "i=" << i << endl;
                //    tsStdOut << "uOffset=" << uOffset << endl;
                //    tsStdOut << "pOffsets-base=" << (char *)pOffsets-(char *)pOffsetsArray << endl;
                //    tsStdOut << "file size=" << qfCurFile->size() << endl;
                //    qFatal("Problem record");
                //}

                // add strobe to database
                db.transaction();
                QByteArray baStrobeHeader(pData,pFileDataHdr->dwSize);
                qint64 iStrobId=addStrobe(uStrobeNo,uBeamCountsNum,baStrobeHeader,iFileId);
                if (iStrobId==-1) {
                    tsStdOut << "\naddStrobe() returned iStrobId==-1" << endl;
                    return 61;
                }
                // tsStdOut << "\naddStrobe() returned iStrobId=" << iStrobId << endl;
                // loop over beams
                for (int iBeam=0; iBeam<iNumberOfBeams; iBeam++) {
                    // tsStdOut << "\t" << iBeam;
                    if (int iRetVal=addSamples(iStrobId,iBeam,(char*)pSamples,uBeamCountsNum*iSizeOfComplex)) {
                        tsStdOut << "\naddSamples() returned:" << iRetVal << endl;
                        return 8;
                    }
                    pSamples+=uBeamCountsNum*2;

                }
                db.commit();
            }
            // unknown format version
            else {
                tsStdOut << "pFileDataHdr->dwType = 0x" << QString::number(pFileDataHdr->dwType,16)
                         << " uFileVersion=" << uFileVersion << endl;
                return 11;
            }
        }
        //############# UNKNOWN record header type ######################
        else {
            tsStdOut << "pFileDataHdr->dwType = 0x" << QString::number(pFileDataHdr->dwType,16) << endl;
            return 12;
        }

        uOffset=*pOffsets++;
        if (pFileDataHdr) delete pFileDataHdr; pFileDataHdr=0;
        if (pData) delete []pData; pData=0;
    }  // Loop over all records within current file
    delete[] pOffsetsArray;
    return 0;
}
//----------------------------------------------------------------
//
//----------------------------------------------------------------
unsigned int getProtoVersion() {
    return uProtoVersion;
}
//----------------------------------------------------------------
//
//----------------------------------------------------------------
unsigned int getFileVersion() {
    return uFileVersion;
}
