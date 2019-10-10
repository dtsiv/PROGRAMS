#ifndef SQLMODEL_H
#define SQLMODEL_H

#include <QtCore>
#include <QtSql>

#define SETTINGS_GROUP_NAME         "PARAMETERS"
#define SETTINGS_KEY_DATAFILE       "dataFile"
#define SETTINGS_KEY_OPERATION      "operation"
#define SETTINGS_KEY_DOPPLERFROM    "dopplerFrom"
#define SETTINGS_KEY_DOPPLERTO      "dopplerTo"
#define SETTINGS_KEY_DELAYFROM      "delayFrom"
#define SETTINGS_KEY_DELAYTO        "delayTo"
#define SETTINGS_KEY_THRESHOLD      "threshold"
#define SETTINGS_KEY_RELTHRESH      "thresholdRelative"
#define SETTINGS_KEY_FALSEALARMS    "falseAlarms"
// interval of time discretization (usec)
#define SETTINGS_KEY_TSAMPL         "samplingTime"
// carrier frequency MHz
#define SETTINGS_KEY_FCARRIER       "carrierF"
// samples per puls
#define SETTINGS_KEY_NTAU           "samplesPerPulse"
// samples per period
#define SETTINGS_KEY_NT             "samplesPerPeriod"
// samples per period recorded
#define SETTINGS_KEY_NT_            "recordedSamplesPerPeriod"
// periods per batch
#define SETTINGS_KEY_NP             "periodsPerBatch"
#define SETTINGS_KEY_DBNAME         "sqliteDbName"
#define SETTINGS_KEY_SLICEPER       "plotSlicePeriod"
#define SETTINGS_KEY_SLICEBEAM      "plotSliceBeam"
#define SETTINGS_KEY_RAWFROM        "rawStrobFrom"
#define SETTINGS_KEY_RAWTO          "rawStrobTo"
#define SETTINGS_KEY_MAXVELO        "maxVelocity"
#define SETTINGS_KEY_PFALARM        "falseAlarmProb"

#define DATA_FILE_NAME              "00000000"

int readSettings();
int openDataFile();
int createTables();
int openDatabase();
int closeDatabase();
qint64 addFileRec(quint64 uTimeStamp, QString qsFilePath, int nStrobs);
qint64 addStrobe(int strobeNo, int beamCountsNum, qint64 iFileId);
int addSamples(qint64 iStrobId, int iBeam, char *pSamples, int iSize);
QString qsGetFileName();

extern enum OperationModes {
    mUndefinedMode=0,
    mDataImport,
    mPrimaryProc,
    mPOI20190409,
    mJustDoppler,
    mPrimaryProcRaw,
    mPrimaryProcNoncoher,
    mSignalPlot,
    mSignalPlot3D
} omSelectedMode;

#endif // SQLMODEL_H
