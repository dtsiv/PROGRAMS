#ifndef QPOI_H
#define QPOI_H

#include <QObject>
#include "qproppages.h"

#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "nr.h"
using namespace std;

#define QPOI_PROP_TAB_CAPTION    "POI"

#define QPOI_SAMPLING_TIME_NSEC             "TSamplingNanoSec"
#define QPOI_CARRIER_FREQUENCY              "CarrierFreqMHz"
#define QPOI_INTFMAP                        "pathIntfMap"
// samples per puls
#define QPOI_NTAU                           "samplesPerPulse"
// samples per period
#define QPOI_NT                             "samplesPerPeriod"
// samples per period recorded
#define QPOI_NT_                            "recordedSamplesPerPeriod"
// periods per batch
#define QPOI_NP                             "periodsPerBatch"
// signal detection threshold
#define QPOI_THRESHOLD                      "threshold"
// false alarm probability
#define QPOI_PFALARM                        "falseAlarmProb"

class QIntfMap;

class QPoi : public QObject {
    Q_OBJECT
public:
    explicit QPoi(QObject *parent = 0);
    ~QPoi();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

    QList<QPointF> detectTargets(QByteArray &baSamples);


signals:

public slots:

private:
    void avevar_poi(Vec_I_DP &data, DP &ave, DP &var);
    void ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob);
    QByteArray dopplerRepresentation(qint16 *pData, unsigned int iArrElemCount);

    double m_dCarrierF;  // Irradiation carrier frequency (MHz)
    double m_dTs;        // sampling time interval (microsecs)
    int NT_;    // NT_ - recorded data samples per period, typically at 20191016 this is 80 samples
    int NT;     // NT - number of samples per period, , typically at 20191016 this 200 samples
    int Np;     // Np - number of periods, typically at 20191016 this is 1024 periods
    int Ntau;   // Ntau - pulse duration (samples), typically at 20191016 this is 8 samples
    int iFilteredN; // in-place array size at filter output: iFilteredN=NT_-Ntau+1; Ntau -pulse duration (samples)
    int NFFT;   // the least power of 2 to cover Np, typically at 20191016 this is 1024 periods
    double m_dThreshold; // signal detection threshold
    double m_dFalseAlarmProb; // probability of false alarm

    QIntfMap *m_pIntfMap;

    friend class QIntfMap;
};

class QIntfMap : public QObject {
    Q_OBJECT
    struct intfMapHeader {
        quint32 vMaj;
        quint32 vMin;
        quint32 Np;
        quint32 NT_;
        quint32 iFilteredN;
        quint32 NFFT;
        quint32 uStrobsCount;
    };

    struct intfMapRecord {
        double iRe;   // average real component of signal in Doppler rep.
        double iIm;   // average imag component
        double uM1;  // average signal modulus
        double uM2;  // average signal modulus2
        double uM3;  // average signal modulus3
        double uM4;  // average signal modulus4
    };

public:
    explicit QIntfMap(QString qsIntfMapFName = "intfmap.dat");
    int readInterferenceMap(unsigned int iFilteredN,unsigned int NT_,unsigned int Np,int NFFT);

    QByteArray m_baAvrRe;
    QByteArray m_baAvrIm;
    QByteArray m_baAvrM1;
    QByteArray m_baAvrM2;
    QByteArray m_baAvrM3;
    QByteArray m_baAvrM4;

    QString m_qsIntfMapFName;

private:
    QFile m_qfIntfMap;
};


#endif // QPOI_H
