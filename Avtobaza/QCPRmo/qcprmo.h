//====================================================================================
// Avtobaza-MR. Cross-platform RMO 
//====================================================================================
#ifndef QCPRMO_H
#define QCPRMO_H
#include "stdafx.h"

#define QCPRMO_VERSION_MAJOR 1
#define QCPRMO_VERSION_MINOR 0

#define QLOCALCONTROLQMLPLUGIN_VERSION "2.04"

#include <QtGui/QMainWindow>

#include "qexceptiondialog.h"
#include "qrmoconnection.h"
#include "rmoexception.h"
#include "qconsolewidget.h"

#include "proppagedlg.h"
#include "proppage.h"
#include "ui_qcprmo.h"
#include "ui_about.h"

#define	IDM_NEW 220
#define	IDM_ADD 221
#define	IDM_OPEN 222
#define	IDM_SAVE 223
#define	IDM_SAVEAS 224
#define	IDM_EXIT 225
#define	IDM_ABOUT 226
#define	IDM_PROP 227
#define	IDM_RULE 228
#define	IDM_CLEAR 229
#define	IDM_PAUSE 230
#define	IDM_VIEW_Q 249
#define	IDM_VIEW_F 250
#define	IDM_VIEW_M 251
#define	IDM_VIEW_T 252
#define	IDM_VIEW_I 253
#define	IDM_VIEW_S 254
#define	IDM_VIEW_C 255

class QCPRmoException : public RmoException {
public:
    QCPRmoException(QString str) : RmoException(QString("QCPRmoEception: " + str)) {}
};

class InitializeForm;
class QCPRmo : public QMainWindow
{
	Q_OBJECT

public:
    QCPRmo(QStringList qslArgs, InitializeForm * pIForm, QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QCPRmo();

private:
    Ui::QCPRmoClass           ui;
    QRmoConnection            *m_pRmoConnection;
    QDomDocument              *m_pDomProp;
    QMap <QString, QObject *> m_rmoWidgets;

private:
    void killRMO();

public:
	void                      reconnect(QString qsAddress, int iPort);
    QString                   m_qsIniFileName;
    QString                   m_Address;
    int                       m_iPort;
    int                       m_PropPageIndx;
    int                       m_iRmoId;
    bool                      m_bConnected;

    QLocalControlQMLWidget * localctrlqml() { return(ui.qLocalControlQMLWidget); }
    QFIndicatorWidget * findicator() { return(ui.qFIndicatorWidget); }
    QConsoleWidget * console() { return(ui.qConsoleWidget);}
    void getConfiguration(QDomDocument * pDomProp);
    void LoadConfiguration(QString qsFile);
    void initialize(QDomDocument * pDomProp);
    void OpenConfiguration();
    void SaveConfiguration(QString qsFile);
    void SaveConfigurationAs();


public slots:
    void onActionTriggered(QAction * pa);
    void addMessage(QString);
    void SendCommand(char * pBuf /* codogram data */, int iSize, int iType);
    void onReceive(QByteArray * pbaPayload, int iType);
	void onConnected(QString sHost, int iPort);
    void onDisconnected();
    void onConsoleClear();
    void onConsolePause();
    void onConsoleSizeChanged(QSize size);
    void onDocPanelVisibilityChanged(bool bVisible);
    void onSendRequest(int iCommId, int iTraceId, double dPar1, double dPar2);
    void onSendFSelectionRequest(int id, double dFSel, double dBandSel, int iCPCount, double dAzFrom, double dAzTo, int iFlags);

};

//*****************************************************************************
//
//*****************************************************************************
class AboutDlg : public QDialog, public Ui::AboutDialog
{
    Q_OBJECT

public:
    AboutDlg(QCPRmo * parent);
};

#endif // QCPRMO_H

