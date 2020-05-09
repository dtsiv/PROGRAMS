#ifndef QNOISEMAP_H
#define QNOISEMAP_H

#include <QtCore>
#include <QFile>

#define NOISE_MAP_VERSION_MAJOR 1
#define NOISE_MAP_VERSION_MINOR 0

class QNoiseMap {
public:
    struct noiseMapHeader {
        quint32 vMaj;
        quint32 vMin;
        quint32 Np;
        quint32 NT_;
        quint32 iFilteredN;
        quint32 NFFT;
        quint32 uStrobsCount;
    };

    struct noiseMapRecord {
        double iRe;   // average real component of signal in Doppler rep.
        double iIm;   // average imag component
        double uM1;  // average signal modulus
        double uM2;  // average signal modulus2
        double uM3;  // average signal modulus3
        double uM4;  // average signal modulus4
    };

public:
    explicit QNoiseMap(QString qsNoiseMapFName = "noisemap.dat");
    int readNoiseMap(unsigned int iFilteredN,unsigned int NT_,unsigned int Np,int NFFT);
    void resizeNoiseAvrArrays(unsigned int iFilteredN,int NFFT,bool bZeroFill = false);
    double getProgress();

    QByteArray m_baAvrRe;
    QByteArray m_baAvrIm;
    QByteArray m_baAvrM1;
    QByteArray m_baAvrM2;
    QByteArray m_baAvrM3;
    QByteArray m_baAvrM4;

    // number to whole strobes processed while generating noise map
    quint32 m_nStrobs;
    // total number of strobes expected
    quint32 m_nStrobsTotal;
    // current number of beams processed (arrays of (qint16,qint16) complex pairs with size NT_)
    quint32 m_nRecs;

    QString m_qsNoisefMapFName;

    QFile m_qfNoiseMap;

    double m_dGlobal_dB;

};

#endif // QNOISEMAP_H
