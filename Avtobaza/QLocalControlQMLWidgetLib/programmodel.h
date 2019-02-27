#ifndef PROGRAMMODEL_H 
#define PROGRAMMODEL_H 

#include <QtDebug>
#include <QObject>
#include <QMessageBox>
#include <QKeySequence>
#include <QMutex>
#include <qdeclarative.h>
#include <QWidget>
#include <QtDesigner/QDesignerExportWidget>
#include <QDomDocument>
#include <QMetaObject>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#ifdef __linux
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#else
    #include "qavtctrl.h"
    #include "codograms.h"
#endif

// highest meaningful bit position is 31, mask 0x80000000, shift 31
#define MAX_SHIFT                       31
// frequency table indices run from 0 to 139
#define MAX_FREQ_IDX                    139
// throughout this code we use index 7 for CND register (this is different from ADU client)
#define REG_IDX_CND			7
// register indices run from 0 to 7, in javascript we adopt notation NA (not assigned) for register index 8 
#define MAX_REG_IDX			REG_IDX_CND

#ifdef __linux
    #define max(a,b) ((a) > (b) ? (a) : (b))
    #define min(a,b) ((a) < (b) ? (a) : (b))
#endif

class ProgramModel : public QObject
{
    Q_OBJECT
//====== object of class ProgramModel is made accessible to QML via QDeclarativeContext::setContextProperty.
    Q_PROPERTY(QString conventions READ conventions WRITE setConventions NOTIFY conventionsChanged)

public:
    ProgramModel(QObject *parent = 0);
    ~ProgramModel();

    Q_INVOKABLE void receiveFrequenciesTable(PFREQUTBL pft, bool askForConfirmation=true);
    Q_INVOKABLE void receiveLocalCtrlInfoEx(PLOCALCTRLSTRUCTEX plci, bool askForConfirmation=true);

    void initApplayLocalCtrlEx(PAPPLAYLOCALCTRLEX pAlc);
    void initFrequencies(PFREQUTBL pft);

    QString conventions(); 
    void initConventions(QString str);
    void setConventions(QString str);

    QString ctlToBase64();
    QString ftblToBase64();

    Q_INVOKABLE void lock(void) { m_Lock.lock(); }
    Q_INVOKABLE void unlock(void) { m_Lock.unlock(); }
    Q_INVOKABLE void submitLocalCtrlEx();
    Q_INVOKABLE void submitFrequTblAtIdx(int freqIdx);
    Q_INVOKABLE void selectTarget();
    Q_INVOKABLE void msg(QString str) { qDebug() << str; /* DbgPrint(str.utf16()); */  }
    Q_INVOKABLE quint32 getRegisterValue(quint32 idx, quint32 mask, quint32 shift);
    Q_INVOKABLE quint32 getFrequencyValue(quint32 idx, quint32 mask, quint32 shift);
    Q_INVOKABLE void setRegisterValue(quint32 idx, quint32 value, quint32 mask, quint32 shift);
    Q_INVOKABLE void setFrequencyValue(quint32 idx, quint32 value, quint32 mask, quint32 shift);
    Q_INVOKABLE int matchShortCut(int key, int modifiers);

private:
    void getFrequTblSize(PFREQUTBL pFtblSrc, int &iStart, int &iCou, int &iSize);

signals:
	//== signals used by the QML engine
    void conventionsChanged();
    void paramsChanged();
    void refreshControls();
	//== signals used within this class to process received codograms
    void localCtrlInfoExReceived(char *, bool);
    void frequenciesReceived(char *, bool);
    void mainCtrlReceived(char *, bool);
	//== signal used to send codograms
    void SendCommand(char *, int, int);
	//== reserved
    void uiParamsChanged();
    void targetSelected(int freqFromHB, int freqToHB);

private slots:
    void onLocalCtrlInfoExReceived(char *, bool);
    void onFrequenciesReceived(char *, bool);

public slots:
    void afterLocalControlQMLWidgetInit();

public:
    QStringList hotKeys;

private:
    QMutex m_Lock;
    // main control structure
    PMAINCTRL          m_pMainCtrl;
    // structure used to receive codograms
    PLOCALCTRLSTRUCTEX m_pCtrl;
    // structure used to send codograms
    PAPPLAYLOCALCTRLEX m_pACtl;
    // frequencies table (read/write)
    PFREQUTBL          m_pFtbl;
    // change flags for frequencies table
    bool               * m_pFreqChng;
    // conventions string (see LOCALCTRLSTRUCTEX)
    QString            m_qsConventions;
};

#endif // PROGRAMMODEL_H 
