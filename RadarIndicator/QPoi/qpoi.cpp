#include "qpoi.h"
#include "qnoisemap.h"
#include "qinisettings.h"
#include "qexceptiondialog.h"

#include "nr.h"
using namespace std;

int QPoi::m_idum = -13;

const char *QPoi::m_pRejectorType[] = {
    "Нет",
    "Узкополосный",
    "ЧПК"
};

//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QPoi::QPoi(QObject *parent)
          : QObject(parent)
          , m_dCarrierF(9500.0e0)
          , m_dTs(0.12e0)  // microseconds
          , m_pNoiseMap(NULL)
          , Np(0)
          , Ntau(0)
          , NT(0)
          , NT_(0)
          , NFFT(0)
          , iBeamCountsNum (0)
          , iSizeOfComplex(2*sizeof(qint16))
          , m_uTargSzThresh(0)
          , m_uMaxTgSize(0)
          , m_dBeamDelta0(0)
          , m_dAntennaSzAz(0)
          , m_dAntennaSzEl(0)
          , m_iWeighting(0)
          , m_pPelengInv(NULL)
          , m_dDeltaBound(0)
          , m_dPelengBound(0)
          , m_dAzLOverD(0)
          , m_dElLOverD(0)
          , m_dPelengIncr(0) {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    QIniSettings::STATUS_CODES scRes;
    iniSettings.setDefault(QPOI_CARRIER_FREQUENCY,9500.0e0);
    m_dCarrierF = iniSettings.value(QPOI_CARRIER_FREQUENCY,scRes).toDouble();
    iniSettings.setDefault(QPOI_SAMPLING_TIME_USEC,0.120e0);
    m_dTs = iniSettings.value(QPOI_SAMPLING_TIME_USEC,scRes).toDouble();
    iniSettings.setDefault(QPOI_NOISEMAP,"noisemap.dat");
    QString qsNoiseMapFName = iniSettings.value(QPOI_NOISEMAP,scRes).toString();
    m_pNoiseMap = new QNoiseMap(qsNoiseMapFName);
    iniSettings.setDefault(QPOI_PFALARM,1.0e-8);
    m_dFalseAlarmProb = iniSettings.value(QPOI_PFALARM,scRes).toDouble();
    iniSettings.setDefault(QPOI_USE_NOISEMAP,true);
    m_bUseNoiseMap = iniSettings.value(QPOI_USE_NOISEMAP,scRes).toBool();
    iniSettings.setDefault(QPOI_PLOT_NOISESPEC,true);
    m_bPlotNoiseSpec = iniSettings.value(QPOI_PLOT_NOISESPEC,scRes).toBool();
    iniSettings.setDefault(QPOI_USE_LOG,true);
    m_bUseLog = iniSettings.value(QPOI_USE_LOG,scRes).toBool();
    iniSettings.setDefault(QPOI_TARGET_SZTHRESH,5);
    m_uTargSzThresh = iniSettings.value(QPOI_TARGET_SZTHRESH,scRes).toInt();
    iniSettings.setDefault(QPOI_TARGET_MAXTGSIZE,20);
    m_uMaxTgSize = iniSettings.value(QPOI_TARGET_MAXTGSIZE,scRes).toInt();
    iniSettings.setDefault(QPOI_BEAM_OFFSET_D0,0.487e0);
    m_dBeamDelta0 = iniSettings.value(QPOI_BEAM_OFFSET_D0,scRes).toDouble();
    iniSettings.setDefault(QPOI_ANTENNA_SIZE_AZ,0.6e0);
    m_dAntennaSzAz = iniSettings.value(QPOI_ANTENNA_SIZE_AZ,scRes).toDouble();
    iniSettings.setDefault(QPOI_ANTENNA_SIZE_EL,0.9e0);
    m_dAntennaSzEl = iniSettings.value(QPOI_ANTENNA_SIZE_EL,scRes).toDouble();
    iniSettings.setDefault(QPOI_ANTENNA_WEIGHTING,QPOI_WEIGHTING_RCOSINE_P065);
    m_iWeighting = iniSettings.value(QPOI_ANTENNA_WEIGHTING,scRes).toUInt();
    iniSettings.setDefault(QPOI_REJECTOR_TYPE,QPOI_REJECTOR_NONE);
    m_iRejectorType = iniSettings.value(QPOI_REJECTOR_TYPE,scRes).toUInt();

    initPeleng();

    resetRandomNumberGenerators();
}
//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
QPoi::~QPoi() {
    QIniSettings &iniSettings = QIniSettings::getInstance();
    iniSettings.setNum(QPOI_CARRIER_FREQUENCY, m_dCarrierF);
    iniSettings.setNum(QPOI_SAMPLING_TIME_USEC, m_dTs);
    if (m_pNoiseMap) iniSettings.setValue(QPOI_NOISEMAP, m_pNoiseMap->m_qsNoisefMapFName);
    iniSettings.setNum(QPOI_PFALARM, m_dFalseAlarmProb);
    iniSettings.setValue(QPOI_USE_NOISEMAP, m_bUseNoiseMap);
    iniSettings.setValue(QPOI_PLOT_NOISESPEC, m_bPlotNoiseSpec);
    iniSettings.setValue(QPOI_USE_LOG, m_bUseLog);
    iniSettings.setNum(QPOI_TARGET_SZTHRESH, m_uTargSzThresh);
    iniSettings.setNum(QPOI_TARGET_MAXTGSIZE, m_uMaxTgSize);
    iniSettings.setNum(QPOI_BEAM_OFFSET_D0, m_dBeamDelta0);
    iniSettings.setNum(QPOI_ANTENNA_SIZE_AZ, m_dAntennaSzAz);
    iniSettings.setNum(QPOI_ANTENNA_SIZE_EL, m_dAntennaSzEl);
    iniSettings.setNum(QPOI_ANTENNA_WEIGHTING, m_iWeighting);
    iniSettings.setNum(QPOI_REJECTOR_TYPE, m_iRejectorType);

    if (m_pNoiseMap) delete m_pNoiseMap;
    if (m_pPelengInv) delete m_pPelengInv;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoi::addTab(QObject *pPropDlg, QObject *pPropTabs, int iIdx) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    QTabWidget *pTabWidget = qobject_cast<QTabWidget *> (pPropTabs);

    QWidget *pWidget=new QWidget;
    QVBoxLayout *pVLayout=new QVBoxLayout;

    QGridLayout *pGLayout=new QGridLayout;
    pGLayout->addWidget(new QLabel("Carrier frequency (MHz)"),0,0);
    pPropPages->m_pleFCarrier = new QLineEdit(QString::number(m_dCarrierF,'f',0));
    pPropPages->m_pleFCarrier->setMaximumWidth(PROP_LINE_WIDTH);
    pGLayout->addWidget(pPropPages->m_pleFCarrier,0,1);

    pGLayout->addWidget(new QLabel("Sampling time (microsec)"),0,3);
    pPropPages->m_pleTSampl = new QLineEdit(QString::number(m_dTs,'f',3));
    pPropPages->m_pleTSampl->setMaximumWidth(PROP_LINE_WIDTH);
    pGLayout->addWidget(pPropPages->m_pleTSampl,0,4);

    pGLayout->addWidget(new QLabel("Target size threshold"),1,0);
    pPropPages->m_pleTargSzThresh = new QLineEdit(QString::number(m_uTargSzThresh));
    pPropPages->m_pleTargSzThresh->setMaximumWidth(PROP_LINE_WIDTH);
    pPropPages->m_pleTargSzThresh->setValidator(new QIntValidator(0,100));
    pGLayout->addWidget(pPropPages->m_pleTargSzThresh,1,1);

    pGLayout->addWidget(new QLabel("Beam relative offset"),1,3);
    pPropPages->m_pleBeamOffsetD0 = new QLineEdit(QString::number(m_dBeamDelta0));
    pPropPages->m_pleBeamOffsetD0->setMaximumWidth(PROP_LINE_WIDTH);
    QLocale locale(QLocale::English);
    QDoubleValidator *pdv;
    pdv = new QDoubleValidator(0,1,3); pdv->setLocale(locale);
    pPropPages->m_pleBeamOffsetD0->setValidator(pdv);
    pGLayout->addWidget(pPropPages->m_pleBeamOffsetD0,1,4);

    pGLayout->addWidget(new QLabel("Antenna size Az (m)"),2,0);
    pPropPages->m_pleAntennaSzAz = new QLineEdit(QString::number(m_dAntennaSzAz));
    pdv = new QDoubleValidator(0,10,2); pdv->setLocale(locale);
    pPropPages->m_pleAntennaSzAz->setValidator(pdv);
    pPropPages->m_pleAntennaSzAz->setMaximumWidth(PROP_LINE_WIDTH);
    pGLayout->addWidget(pPropPages->m_pleAntennaSzAz,2,1);

    pGLayout->addWidget(new QLabel("Antenna size El (m)"),2,3);
    pPropPages->m_pleAntennaSzEl = new QLineEdit(QString::number(m_dAntennaSzEl));
    pPropPages->m_pleAntennaSzEl->setMaximumWidth(PROP_LINE_WIDTH);
    pdv = new QDoubleValidator(0,10,2); pdv->setLocale(locale);
    pPropPages->m_pleAntennaSzEl->setValidator(pdv);
    pGLayout->addWidget(pPropPages->m_pleAntennaSzEl,2,4);

    pGLayout->addWidget(new QLabel("Antenna weighting"),3,0);
    pPropPages->m_pcbbWeighting=new QComboBox;
    pPropPages->m_pcbbWeighting->addItem(m_pWeightingType[QPOI_WEIGHTING_RCOSINE_P065]);
    pPropPages->m_pcbbWeighting->setCurrentIndex(m_iWeighting);
    pGLayout->addWidget(pPropPages->m_pcbbWeighting,3,1);

    pGLayout->addWidget(new QLabel("Interference rejection"),3,3);
    pPropPages->m_pcbbRejector=new QComboBox;
    for (int i=0; i<sizeof(m_pRejectorType)/sizeof(m_pRejectorType[0]); i++) {
        pPropPages->m_pcbbRejector->addItem(QString::fromUtf8(m_pRejectorType[i]));
    }
    pPropPages->m_pcbbRejector->setCurrentIndex(m_iRejectorType);
    pGLayout->addWidget(pPropPages->m_pcbbRejector,3,4);

    pGLayout->addWidget(new QLabel("False alarm probability"),4,0);
    pPropPages->m_pleFalseAlarmP = new QLineEdit(QString::number(m_dFalseAlarmProb,'e',1));
    pdv = new QDoubleValidator(0,1,2); pdv->setLocale(locale);
    pPropPages->m_pleAntennaSzEl->setValidator(pdv);
    pPropPages->m_pleFalseAlarmP->setMaximumWidth(PROP_LINE_WIDTH);
    pGLayout->addWidget(pPropPages->m_pleFalseAlarmP,4,1);

    pGLayout->addWidget(new QLabel("Max target size"),4,3);
    pPropPages->m_pleMaxTgSize = new QLineEdit(QString::number(m_uMaxTgSize));
    pPropPages->m_pleMaxTgSize->setMaximumWidth(PROP_LINE_WIDTH);
    pPropPages->m_pleMaxTgSize->setValidator(new QIntValidator(0,100));
    pGLayout->addWidget(pPropPages->m_pleMaxTgSize,4,4);

    pGLayout->setColumnStretch(2,100);
    pGLayout->setColumnStretch(5,100);
    pVLayout->addLayout(pGLayout);

    QHBoxLayout *pHLayout01=new QHBoxLayout;
    pPropPages->m_pleNoiseMapFName=new QLineEdit(m_pNoiseMap->m_qsNoisefMapFName);
    pHLayout01->addWidget(new QLabel("Noise map file"));
    pHLayout01->addWidget(pPropPages->m_pleNoiseMapFName);
    QPushButton *ppbChoose=new QPushButton(QIcon(":/Resources/open.ico"),"Choose");
    QObject::connect(ppbChoose,SIGNAL(clicked()),pPropPages,SIGNAL(chooseNoiseMapFile()));
    QObject::connect(pPropPages,SIGNAL(chooseNoiseMapFile()),SLOT(onNoiseMapFileChoose()));
    pHLayout01->addWidget(ppbChoose);
    pVLayout->addLayout(pHLayout01);

    QHBoxLayout *pHLayout02=new QHBoxLayout;
    pPropPages->m_pcbUseNoiseMap=new QCheckBox("Use noise map");
    pPropPages->m_pcbUseNoiseMap->setChecked(m_bUseNoiseMap);
    pHLayout02->addWidget(pPropPages->m_pcbUseNoiseMap);
    pPropPages->m_ppbGenerateNoiseMap=new QPushButton("Generate");
    QObject::connect(pPropPages->m_ppbGenerateNoiseMap,SIGNAL(clicked()),pPropPages,SIGNAL(generateNoiseMapFile()));
    pHLayout02->addWidget(pPropPages->m_ppbGenerateNoiseMap);
    pPropPages->m_ppbarGenerateNoiseMap=new QProgressBar;
    pPropPages->m_ppbarGenerateNoiseMap->setValue(0);
    pHLayout02->addWidget(pPropPages->m_ppbarGenerateNoiseMap);
    pVLayout->addLayout(pHLayout02);

    pVLayout->addStretch();
    pWidget->setLayout(pVLayout);
    pTabWidget->insertTab(iIdx,pWidget,QPOI_PROP_TAB_CAPTION);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPoi::propChanged(QObject *pPropDlg) {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (pPropDlg);
    bool bOk;
    QString qs;

    qs = pPropPages->m_pleFCarrier->text();
    double dCarrierF = qs.toDouble(&bOk);
    if (bOk) m_dCarrierF = qBound(5000.0e0,dCarrierF,15000.0e0);

    qs = pPropPages->m_pleTSampl->text();
    double dTs = qs.toDouble(&bOk);
    if (bOk) m_dTs = qBound(0.01e0,dTs,10.0e0); // microseconds

    qs = pPropPages->m_pleTargSzThresh->text();
    quint32 uTargSzThresh = qs.toUInt(&bOk);
    if (bOk) m_uTargSzThresh = qMax((quint32)0,uTargSzThresh);

    qs = pPropPages->m_pleBeamOffsetD0->text();
    double dBeamDelta0 = qs.toDouble(&bOk);
    if (bOk) m_dBeamDelta0 = qMax(0.0e0,dBeamDelta0);

    qs = pPropPages->m_pleAntennaSzAz->text();
    double dAntennaSzAz = qs.toDouble(&bOk);
    if (bOk) m_dAntennaSzAz = qMax(0.0e0,dAntennaSzAz);

    qs = pPropPages->m_pleAntennaSzEl->text();
    double dAntennaSzEl = qs.toDouble(&bOk);
    if (bOk) m_dAntennaSzEl = qMax(0.0e0,dAntennaSzEl);

    m_iWeighting = pPropPages->m_pcbbWeighting->currentIndex();

    qs = pPropPages->m_pleMaxTgSize->text();
    quint32 uMaxTgSize = qs.toUInt(&bOk);
    if (bOk) m_uMaxTgSize = qMax((quint32)0,uMaxTgSize);

    m_iRejectorType = pPropPages->m_pcbbRejector->currentIndex();

    qs = pPropPages->m_pleFalseAlarmP->text();
    double dFalseAlarmP = qs.toDouble(&bOk);
    if (bOk) m_dFalseAlarmProb = qMax(0.0e0,dFalseAlarmP);

    m_pNoiseMap->m_qsNoisefMapFName = pPropPages->m_pleNoiseMapFName->text();
    m_bUseNoiseMap = pPropPages->m_pcbUseNoiseMap->isChecked();
}
//======================================================================================================
//
//======================================================================================================
void QPoi::resetRandomNumberGenerators() {
    // initialize seed of stdlib.h pseudo-random numbers (iSeed is read-only)
    unsigned int iSeed=1;
    std::srand(iSeed);

    // initialize seed of Numerical Recipes Gaussian random numbers (m_idum is read-write)
    this->m_idum=-1;
    NR::gasdev(this->m_idum);
}
//====================================================================
//
//====================================================================
void QPoi::avevar_poi(Vec_I_DP &data, DP &ave, DP &var) {
    DP s;
    int j;

    int n=data.size();
    ave=0.0;
    var=0.0;
    for (j=0;j<n;j++) {
        s=data[j]-ave;
        var += s*s;
    }
    var=var/n;
}
//====================================================================
//
//====================================================================
void QPoi::onNoiseMapFileChoose() {
    QPropPages *pPropPages = qobject_cast<QPropPages *> (QObject::sender());

    QFileDialog dialog(pPropPages);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Noise map file (*.dat)"));
    // if current file exists - set its directory in dialog
    QString qsCurrFile = pPropPages->m_pleNoiseMapFName->text();
    QFileInfo fiCurrFile(qsCurrFile);
    if (fiCurrFile.absoluteDir().exists()) {
        dialog.setDirectory(fiCurrFile.absoluteDir());
    }
    else { // otherwise set current directory in dialog
        dialog.setDirectory(QDir::current());
    }
    if (dialog.exec()) {
        QStringList qsSelection=dialog.selectedFiles();
        if (qsSelection.size() != 1) return;
        QString qsSelFilePath=qsSelection.at(0);
        QFileInfo fiSelFilePath(qsSelFilePath);
        if (fiSelFilePath.isFile() && fiSelFilePath.isReadable())	{
            QDir qdCur = QDir::current();
            QDir qdSelFilePath = fiSelFilePath.absoluteDir();
            if (qdCur.rootPath() == qdSelFilePath.rootPath()) { // same Windows drives
                pPropPages->m_pleNoiseMapFName->setText(qdCur.relativeFilePath(fiSelFilePath.absoluteFilePath()));
            }
            else { // different Windows drives
                pPropPages->m_pleNoiseMapFName->setText(fiSelFilePath.absoluteFilePath());
            }
        }
        else {
            pPropPages->m_pleNoiseMapFName->setText("error");
        }
    }
}
