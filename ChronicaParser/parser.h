#ifndef PARSER_H
#define PARSER_H

#include "qchrprotoacm.h"
#include "sqlmodel.h"

int parseDataFile(quint64 uTimeStamp,QString qsFilePath, char *pFileData, quint64 uSize);

struct sFileHdr {
    char sMagic[4];
    quint32 uVer;
    quint32 nRecMax;
    quint32 nRec;
};

extern struct sFileHdr fileHdr;

#endif // PARSER_H
