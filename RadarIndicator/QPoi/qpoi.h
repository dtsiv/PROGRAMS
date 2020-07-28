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

#include "qnoisemap.h"

#define QPOI_PROP_TAB_CAPTION    "POI"

#define QPOI_SAMPLING_TIME_USEC             "TSamplingMicroSec"
#define QPOI_CARRIER_FREQUENCY              "CarrierFreqMHz"
#define QPOI_NOISEMAP                       "pathNoiseMap"
// signal detection threshold
#define QPOI_THRESHOLD                      "threshold"
// false alarm probability
#define QPOI_PFALARM                        "falseAlarmProb"
// use noise scaling in doppler domain
#define QPOI_USE_NOISEMAP                   "useNoiseMap"
// output noise spectrum for plotting
#define QPOI_PLOT_NOISESPEC                 "plotNoiseSpec"
// output simulation log
#define QPOI_USE_LOG                        "simulationLog"
// detection threshold on target size
#define QPOI_TARGET_SZTHRESH                "tarSizeThresh"
// beam relative offset Delta_0 from scan line in each direction (see manuscript for details)
#define QPOI_BEAM_OFFSET_D0                 "beamOffsetD0"
// antenna size along Az dir in meters
#define QPOI_ANTENNA_SIZE_AZ                "antennaSizeAz"
// antenna size along El dir in meters
#define QPOI_ANTENNA_SIZE_EL                "antennaSizeEl"
// antenna weighting type
#define QPOI_ANTENNA_WEIGHTING              "antennaWeight"

#define QPOI_WEIGHTING_RCOSINE_P065         0

#define QPOI_NUMBER_OF_BEAMS                4
#define QPOI_MAXIMUM_TG_MISMATCH            10

#define GENERATE_PROGRESS_BAR_STEP          1
#define GENERATE_PROGRESS_BAR_MAX           100

class QNoiseMap;

class QPoi : public QObject {
    Q_OBJECT
public:
    explicit QPoi(QObject *parent = 0);
    ~QPoi();

    Q_INVOKABLE void addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx);
    Q_INVOKABLE void propChanged(QObject *pPropDlg);

    bool detectTargets(QByteArray &baSamplesDP, QByteArray &baStrTargets, int &nTargets);

    struct sTarget {
        unsigned int uCandNum;   // number of candidates attributed to this target
        QPointF      qpf_wei;    // (dimensional) mass center for target delay index l, Doppler index k (0,...,NFFT)
                                 // weighted over candidates (with y2mc) and multiplied by dDistCoef, dVelCoef
        double       y2mc_sum;   // total energy of target
        QPoint       qp_rep;     // representative candidate resolution element (lc_rep, kc_rep)
        double       y2mc_rep;   // energy for representative candidate
    };

    // random seed
    static int m_idum;

signals:

public slots:
    void onNoiseMapFileChoose();

public:
    void avevar_poi(Vec_I_DP &data, DP &ave, DP &var);
    // void ftest_poi(Vec_I_DP &data_noise, Vec_I_DP &data_signal, DP &f, DP &prob);
    QByteArray dopplerRepresentation(QByteArray &baSamplesDP);
    bool getPointDopplerRep(int iDelay, int kDoppler,
        QByteArray pbaBeamDataDP[QPOI_NUMBER_OF_BEAMS],
        double dBeamAmplRe[QPOI_NUMBER_OF_BEAMS],
        double dBeamAmplIm[QPOI_NUMBER_OF_BEAMS]);
    void resetRandomNumberGenerators();
    bool checkStrobeParams(int iNp,              /* pStructStrobeHeader->pCount */
                           int iNtau,            /* pStructStrobeHeader->pDuration */
                           int iNT,              /* pStructStrobeHeader->pPeriod */
                           int iNT_,             /* pStructStrobeHeader->distance */
                           int iBeamCountsNum    /* beamCountsNum = NT_ * Np */
                           );
    bool appendStrobToNoiseMap(QByteArray baSamples[QPOI_NUMBER_OF_BEAMS]);
    bool calcAverages();
    bool updateNFFT();
    bool updateStrobeParams(int iNp,              /* pStructStrobeHeader->pCount */
                            int iNtau,            /* pStructStrobeHeader->pDuration */
                            int iNT,              /* pStructStrobeHeader->pPeriod */
                            int iNT_,             /* pStructStrobeHeader->distance */
                            int iBeamCountsNum    /* beamCountsNum = NT_ * Np */
                            );
    void writeNoiseMapFile(QString qsFileName);
    bool getAngles(double dBeamAmplRe[4], double dBeamAmplIm[4], double &dAzimuth, double &dElevation);
    double dsinc(double x);
    double raised_cos_spec(double Delta, double p=0.65);
    double raised_cos_charact(double Delta);
    double dichotomy(double dFuncValue, double dLeft, double dRight, double (QPoi::*pFunc)(double), bool *pbOk = NULL);
    void initPelengInv(double (QPoi::*pFunc)(double));
    void initPeleng();
    double getRelAngOffset(double dPelengRatio, bool *pbOk = NULL);

public:
    double m_dCarrierF;  // Irradiation carrier frequency (MHz)
    double m_dTs;        // sampling time interval (microsecs)
    int NT_;    // NT_ - recorded data samples per period, typically at 20191016 this is 80 samples
    int NT;     // NT - number of samples per period, , typically at 20191016 this 200 samples
    int Np;     // Np - number of periods, typically at 20191016 this is 1024 periods
    int Ntau;   // Ntau - pulse duration (samples), typically at 20191016 this is 8 samples
    int iFilteredN; // in-place array size at filter output: iFilteredN=NT_-Ntau+1; Ntau -pulse duration (samples)
                    // 20200502: changed to iFilteredN=NT_
    int NFFT;   // the least power of 2 to cover Np, typically at 20191016 this is 1024 periods
    int iBeamCountsNum; // number of complex samples per beam = NT_*Np
    double m_dFalseAlarmProb; // probability of false alarm
    bool m_bUseNoiseMap;
    bool m_bPlotNoiseSpec;
    bool m_bUseLog;
    QByteArray m_baStructFileHdr;
    QByteArray m_baStructStobeData;
    quint32 m_uFileVer;

    QNoiseMap *m_pNoiseMap;
    int iSizeOfComplex;

    quint32 m_uTargSzThresh;

    // Peleng-related
    double m_dBeamDelta0;
    double m_dAntennaSzAz;
    double m_dAntennaSzEl;
    int m_iWeighting;
    static const char *m_pWeightingType[];
    double m_dAzLOverD;
    double m_dElLOverD;
    double *m_pPelengInv;
    double m_dDeltaBound;
    double m_dPelengBound;
    double m_dPelengIncr;

    friend class QNoiseMap;
};

#endif // QPOI_H
