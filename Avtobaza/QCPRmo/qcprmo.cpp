#include "stdafx.h"
#include <QAbstractSocket>
#include <QDomDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QByteArray>

#include "qcprmo.h"
#include "initialize.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//*****************************************************************************
//
//*****************************************************************************
QCPRmo::QCPRmo(QStringList qslArgs, InitializeForm * pIForm,
       QWidget *parent /* = 0 */, Qt::WFlags flags /* = 0 */ )
        : QMainWindow(parent, flags)
        , m_pRmoConnection(0)
        , m_pDomProp (0) {
    try {
        ui.setupUi(this);
    }
    catch (std::exception & e) {
        showExceptionDialog(QString("During QCPRmo ui.setupUi: ") + e.what());
    }
    setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));
    QString qsQmlPluginVersion;
    if (!QMetaObject::invokeMethod(
            this->localctrlqml(),"version", Qt::DirectConnection,
            Q_RETURN_ARG(QString, qsQmlPluginVersion))) {
        QMessageBox::critical(parent,"call to qml plugin version()","QMetaObject::invokeMethod failed!");
        killRMO();
        return;
    }
    if (qsQmlPluginVersion != QLOCALCONTROLQMLPLUGIN_VERSION) {
        QString qs("Invalid QLocalControlQMLPlugin version %1. Expected: %2");
        QMessageBox::critical(
            parent,tr("VersionMismatch"),qs.arg(qsQmlPluginVersion).arg(QLOCALCONTROLQMLPLUGIN_VERSION)
        );
        killRMO();
        return;
    }
    m_PropPageIndx = 0;
    m_iRmoId = 77;
    m_qsIniFileName = QDir::current().filePath("rmodefault.xml");
    m_Address = "127.0.0.1";
    m_iPort = 7776;
    m_bConnected = false;
    m_pDomProp = new QDomDocument("RmoProperties");

    pIForm->onSetLoadStatus(QString("Main RMO application:100"));

    LoadConfiguration(m_qsIniFileName);

    setWindowTitle("QCPRmo-" + m_qsIniFileName);

    m_rmoWidgets.insert(QString("LocalControlQMLWidget"),this->localctrlqml());
    // initialize QRmoConnection object
    m_pRmoConnection = new QRmoConnection(m_Address,m_iPort);
    // m_pLocalControlQMLWidget SendCommand signal
    // from the local control panel must block
    // until the codogram is successfully sent
    QObject::connect(
        this->localctrlqml(), // m_pLocalControlQMLWidget is on the same thread as this object
        SIGNAL(SendCommand(char*,int,int)),
        SLOT(SendCommand(char*,int,int)),
        Qt::DirectConnection);
    // connect the onReceive slot
    QObject::connect(
        m_pRmoConnection, 
        SIGNAL(receive(QByteArray*,int)),
        SLOT(onReceive(QByteArray*,int)),
		QRmoConnection::UniqueBlockingQueuedConnection);
    // onConnected
    QObject::connect(
        m_pRmoConnection, 
        SIGNAL(connected(QString,int)),
        SLOT(onConnected(QString,int)));
    // onDisconnected
    QObject::connect(
        m_pRmoConnection,
        SIGNAL(disconnected()),
        SLOT(onDisconnected()));
}

//*****************************************************************************
//
//*****************************************************************************
QCPRmo::~QCPRmo() {
    if (!m_qsIniFileName.isEmpty()) SaveConfiguration(m_qsIniFileName);
    try {
        if (m_pRmoConnection) delete m_pRmoConnection;
    }
    catch (std::exception &e) {
        showExceptionDialog(QString("QRmoConnection failed to destroy itself: ") + e.what());
    }
    if (m_pDomProp) delete m_pDomProp;
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::reconnect(QString qsAddress, int iPort) { 
	if (!m_pRmoConnection) {
		qDebug() << "QCPRmo::reconnect(). m_pRmoConnection was not initialized";
		return;
	}

	//===== OPTION 1: use QRmoConnection::reconnect()
	// if (m_pRmoConnection) m_pRmoConnection->reconnect(qsAddress, iPort);

	//===== OPTION 2: use waitForDisconnected()
	// doDisconnect() is non-blocking
	m_pRmoConnection->doDisconnect();
	// Let the receive slot process all queued QRmoConnection::receive() signals
	QCoreApplication::processEvents();
	if (m_pRmoConnection->waitForDisconnected(30000)) {
        delete m_pRmoConnection;
		qDebug() << "QCPRmo::reconnect(). delete m_pRmoConnection successful";
		m_pRmoConnection = new QRmoConnection(qsAddress,iPort);
	}
	else { // this situation is incorrect
		qDebug() << "QCPRmo::reconnect() timeout";
	}
}

//*****************************************************************************
//    this slot has Qt::DirectConnection. It is executed in widget's thread
//*****************************************************************************
void QCPRmo::SendCommand(
	char * pBuf /* codogram data */,
	int iSize /* codogram size */,
	int iType /* codogram type */) {
    // construct QByteArray from codogram data - NO DEEP COPY
	QByteArray baPayload = QByteArray::fromRawData(pBuf, iSize);
    // send blocks until success or error
	m_pRmoConnection->send(&baPayload,iType);
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onReceive(QByteArray * pbaPayload, int iType) {
	// get pointer to QLocalControlQMLWidget
	QLocalControlQMLWidget * pLocalControlQMLWidget = localctrlqml();
        QFIndicatorWidget * pFIndicator = findicator();
	char * pBuf = pbaPayload->data();
	int iSize = pbaPayload->size();

	// qDebug() << "QCPRmo::onReceive: Received iSize: " << iSize << ". Type: " << iType;

	// dispatch CTLINFOEX codogram 
    if (iType == WTYP_CTLINFOEX) {
        emit addMessage(QString("Received codogram WTYP_CTLINFOEX iType=%1 iSize=%2 sizeof(LOCALCTRLSTRUCTEX)=%4")
                        .arg(iType).arg(iSize).arg(sizeof(LOCALCTRLSTRUCTEX)));
        // invoke receiveLocalCtrlInfoEx() method by its name. the return value is of no interest
        if (pLocalControlQMLWidget) {
            QMetaObject::invokeMethod(
				pLocalControlQMLWidget,
				"receiveLocalCtrlInfoEx",
				Qt::DirectConnection, // need synchronous call
				Q_ARG(PLOCALCTRLSTRUCTEX,(PLOCALCTRLSTRUCTEX)pBuf),
				Q_ARG(bool,false));   // bAskForConfirmation
			}
	}
	// dispatch FREQUCHANGE codogram 
    if(iType == WTYP_FREQUCHANGE) {
        emit addMessage(QString("Received codogram WTYP_FREQUCHANGE iType=%1 iSize=%2").arg(iType).arg(iSize));
        // invoke receiveFrequenciesTable() method by its name. the return value is of no interest
        if (pLocalControlQMLWidget) {
			QMetaObject::invokeMethod(
				pLocalControlQMLWidget,
				"receiveFrequenciesTable",
				Qt::DirectConnection, // need synchronous call
				Q_ARG(PFREQUTBL,(PFREQUTBL)pBuf),
				Q_ARG(bool,false));   // bAskForConfirmation
        }
    }
    if(iType == WTYP_TRACELIST) {
        if (pFIndicator) {
            int iCou = iSize / sizeof(TRACELISTENTRY);
	    QMetaObject::invokeMethod(
		pFIndicator,
		"traceTbl",
		Qt::DirectConnection, // need synchronous call
		Q_ARG(PTRACELISTENTRY,(PTRACELISTENTRY)pBuf),
		Q_ARG(int,iCou));
        }
    }
    if(iType == WTYP_POIT     ) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"AddPoit",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PPOIT,(PPOIT)pBuf));
        }
    }
    if(iType == WTYP_POITE    ) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"AddPoit",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PPOITE,(PPOITE)pBuf));
        }
    }
    if(iType == WTYP_TRACEINFO) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"traceInfo",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PTRACEINFOHEADER,(PTRACEINFOHEADER)pBuf));
        }
    }
    if(iType == WTYP_TRACEINFOE) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"traceInfoE",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PTRACEINFOHEADER,(PTRACEINFOHEADER)pBuf));
        }
    }
    if(iType == WTYP_POSTT    ) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"AddPostt",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PPOSTT,(PPOSTT)pBuf));
        }
    }
    if(iType == WTYP_SCOPE16C ) {
        if (pFIndicator) {
	    QMetaObject::invokeMethod(
		pFIndicator,
		"AddRaw16",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(PSCOPE16C,(PSCOPE16C)pBuf));
        }
    }
    if(iType == WTYP_FRSELECTION) {
        if (pFIndicator) {
            int iCou = iSize / sizeof(FRSELECTION);
	    QMetaObject::invokeMethod(
		pFIndicator,
		"updateFrSel",
		Qt::DirectConnection, // need synchronous call
                Q_ARG(int,m_iRmoId),
                Q_ARG(PFRSELECTION,(PFRSELECTION)pBuf),
                Q_ARG(int,iCou));
        }
    }
/*
    if(iType == WTYP_TRACELIST) emit addMessage(QString("Received codogram WTYP_TRACELIST"));
    if(iType == WTYP_POIT     ) emit addMessage(QString("Received codogram WTYP_POIT     "));
    if(iType == WTYP_POITE    ) emit addMessage(QString("Received codogram WTYP_POITE    "));
    if(iType == WTYP_TRACEINFO) emit addMessage(QString("Received codogram WTYP_TRACEINFO"));
    if(iType == WTYP_TRACEINFOE) emit addMessage(QString("Received codogram WTYP_TRACEINFOE"));
    if(iType == WTYP_POSTT    ) emit addMessage(QString("Received codogram WTYP_POSTT    "));
    if(iType == WTYP_SCOPE16C ) emit addMessage(QString("Received codogram WTYP_SCOPE16C "));
    if(iType == WTYP_FRSELECTION) emit addMessage(QString("Received codogram WTYP_FRSELECTION"));
*/
    if(iType == WTYP_TRACELIST
    || iType == WTYP_POIT     
    || iType == WTYP_POITE    
    || iType == WTYP_TRACEINFO
    || iType == WTYP_TRACEINFOE
    || iType == WTYP_POSTT    
    || iType == WTYP_SCOPE16C 
    || iType == WTYP_FRSELECTION) 
        qDebug() << "Received codogram " << iType;

	// payload was dispatched. Delete it (otherwise memory leaks!)
    delete pbaPayload;
}


//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onSendRequest(int iCommId, int iTraceId, double dPar1, double dPar2) {
    // DbgPrint(L"QRmo: OnSendRequest id=%ld trace=%ld par1=%lf", iCommId, iTraceId, dPar1);
    if(!m_pRmoConnection->isConnected()) return;

    RMOCOMMAND rmocom;
    rmocom.Id = (unsigned long)iCommId;
    rmocom.uNum = (unsigned long)iTraceId;
    rmocom.rm = (unsigned long)m_iRmoId;        // номер рабочего места
    rmocom.uFlags = 0;                          // Err code
    rmocom.dLat = dPar1;
    rmocom.dLon = dPar2;

    int iSize = sizeof(RMOCOMMAND);             // codogram size
    int iType = WTYP_VOICOM;                    // codogram type
    QByteArray baPayload = QByteArray::fromRawData((const char *)&rmocom, iSize);
    m_pRmoConnection->send(&baPayload,iType);
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onDocPanelVisibilityChanged(bool /* bVisible */ ) {
    QAction *pa;
    QDockWidget * pdw = (QDockWidget *) sender();

    if (pdw==ui.dockWidgetF) pa = ui.actionViewFIndicator;
    else if (pdw==ui.dockWidgetM) pa = ui.actionViewMessages;
    else if (pdw==ui.dockWidgetQ) pa = ui.actionControlPanelQML;
    else return;

    pa -> setChecked(!pdw -> isHidden());
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onConsoleClear() {
    console() -> clear();
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onConsolePause() {
    console()->pause(ui.toolButtonPause -> isChecked());
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onConsoleSizeChanged(QSize size) {
	QScrollBar * psb = ui.scrollAreaM -> verticalScrollBar();
	int val = psb -> maximum();
	psb -> setValue(val);
	psb -> update();
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onSendFSelectionRequest(int id, double dFSel,
               double dBandSel, int iCPCount,
               double dAzFrom, double dAzTo, int iFlags) {
    if(!m_pRmoConnection->isConnected()) return;

    FRSELECTION frsel;
	frsel.rm = (unsigned long)m_iRmoId;
	frsel.id = (unsigned long)id;
	frsel.dFSel = dFSel;
	frsel.dBandSel = dBandSel;
	frsel.dAzFrom = dAzFrom;
	frsel.dAzTo = dAzTo;
	frsel.iCPCount = iCPCount;
	frsel.iFlags = iFlags;
	frsel.iReserved[0] = 0;
	frsel.iReserved[1] = 0;

    int iSize = sizeof(FRSELECTION);             // codogram size
    int iType = WTYP_FRSELECTION;                // codogram type
    QByteArray baPayload = QByteArray::fromRawData((const char *)&frsel, iSize);
    m_pRmoConnection->send(&baPayload,iType);
    // DbgPrint(L"QRmo: OnSendFSelectionRequest id=%ld azFrom=%0.1lf azTo=%0.1lf F=%0.3lf", frsel.id, frsel.dAzFrom, frsel.dAzTo, frsel.dFSel);
}
//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::killRMO() {
    QTimer::singleShot(0,this,SLOT(close()));
}
//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onConnected(QString sHost, int iPort) {
    QString qsCon=QString("QCPRmo connected to %1:%2").arg(sHost).arg(iPort);
    emit addMessage(qsCon);
    setWindowTitle(qsCon);
}
//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onDisconnected() {
    QString qsCon=QString("QCPRmo disconnected");
    emit addMessage(qsCon);
    setWindowTitle(qsCon);
}
//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::onActionTriggered(QAction * pa) {
    QVariant qv = pa -> property("menuId");

    RmoPropDlg * ppd;
    QWidget * pwg;
    QList<QWidget *> * pwl;

    int iRet;

    switch(qv.toInt())
    {

        case IDM_OPEN:
            OpenConfiguration();
            break;

        case IDM_SAVE:
            SaveConfiguration(m_qsIniFileName);
            break;

        case IDM_SAVEAS:
            SaveConfigurationAs();
            break;

        case IDM_EXIT:
            close();
            break;

        case IDM_ABOUT:
            {	AboutDlg * pad = new AboutDlg(this);
                pad -> exec();
                delete pad;
            }
            break;

        case IDM_PROP:
            pwl = new QList<QWidget *>;
            pwg = (QWidget *)new RmoPropPage(this, this);
            pwl -> append(pwg);
            this->localctrlqml() -> getPropPages(pwl, this);
            this->findicator()   -> getPropPages(pwl, this);
            this->console()      -> getPropPages(pwl, this);
            ppd = new RmoPropDlg(this);
            ppd -> AddPages(pwl);
            iRet = ppd -> exec();
            delete ppd;
            delete pwl;
            QCoreApplication::instance() -> processEvents();
            /*
            if(iRet & APPLAYCHANGEFLAG_TRANSFORMER)
            {	setUpdatesEnabled(false);
                InitializeForm * initform = new InitializeForm(NULL);
                initform -> show();
                QObject::connect(ui.qGeoIndicatorWidget -> map(), SIGNAL(SetLoadStatus(QString)),
                    initform, SLOT(OnSetLoadStatus(QString)));
                QCoreApplication::instance() -> processEvents();
                ui.qGeoIndicatorWidget -> ReprojMap();
                QObject::disconnect(initform, SLOT(OnSetLoadStatus(QString)));
                delete initform;
                setUpdatesEnabled(true);
            }
            */
            break;

        case IDM_CLEAR:
            ui.qFIndicatorWidget -> clear();
            break;

        case IDM_PAUSE:
            break;

        case IDM_VIEW_F:
            if(pa -> isChecked()) ui.dockWidgetF -> show();
            else ui.dockWidgetF -> hide();
            break;

        case IDM_VIEW_Q:
            if(pa -> isChecked()) ui.dockWidgetContentsQ -> show();
            else ui.dockWidgetContentsQ -> hide();
            break;

        default:
            // DbgPrint(L"QRmo: unknown menu id=%ld", qv.toInt());
            throw RmoException("Unknown action");
            break;
    }
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::SaveConfiguration(QString qsFile)
{
    getConfiguration(m_pDomProp);
    QFile file(qsFile);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {	QTextStream stream(&file);
        stream.setFieldAlignment(QTextStream::AlignLeft);
        m_pDomProp -> save(stream, 2, QDomNode::EncodingFromDocument);
        file.close();
    } else
        qDebug() << "QRmo: Saving configuration failed file=" << qsFile;
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::OpenConfiguration()
{	QStringList qslFilters;
    QFileDialog * pfd;
    QString qsMap = "Map files (*.m41)";
    QString qsXml = "Rmo config files (*.xml)";

    pfd = new QFileDialog(this, QString("Open configuration"));
    pfd -> setFileMode(QFileDialog::ExistingFile);
    pfd -> setAcceptMode(QFileDialog::AcceptOpen);
    pfd -> setDefaultSuffix(QString("xml"));
    pfd -> setViewMode(QFileDialog::Detail);
    qslFilters << qsXml << qsMap << "All files (*.*)";
    pfd -> setNameFilters(qslFilters);

    if(!pfd -> exec())
    {	delete pfd;
        return;
    }
    QCoreApplication::instance() -> processEvents();
    ui.menuBar -> setEnabled(false);
    ui.mainToolBar -> setEnabled(false);
    QCoreApplication::instance() -> processEvents();
    if(pfd -> selectedNameFilter() == qsMap)
    {	// ui.qGeoIndicatorWidget -> LoadMap(pfd -> selectedFiles().at(0));
    } else
    if(pfd -> selectedNameFilter() == qsXml)
    {	m_qsIniFileName = pfd -> selectedFiles().at(0);
        LoadConfiguration(m_qsIniFileName);
    }
    setWindowTitle("QRmo-" + m_qsIniFileName);
    ui.menuBar -> setEnabled(true);
    ui.mainToolBar -> setEnabled(true);
    delete pfd;
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::SaveConfigurationAs()
{	QStringList qslFilters;
    QFileDialog * pfd;
    QString qsMap = "Map files (*.m41)";
    QString qsXml = "Rmo config files (*.xml)";

    pfd = new QFileDialog(this, QString((QChar *)L"Save configuration"));
    pfd -> setFileMode(QFileDialog::AnyFile);
    pfd -> setAcceptMode(QFileDialog::AcceptSave);
    pfd -> setDefaultSuffix(QString("xml"));
    pfd -> setViewMode(QFileDialog::Detail);
    qslFilters << qsXml << qsMap << "All files (*.*)";
    pfd -> setNameFilters(qslFilters);
    pfd -> selectFile(m_qsIniFileName);

    if(!pfd -> exec())
    {	delete pfd;
        return;
    }
    if(pfd -> selectedNameFilter() == qsMap)
    {
        // ui.qGeoIndicatorWidget -> SaveMap(pfd -> selectedFiles().at(0));

    } else
    if(pfd -> selectedNameFilter() == qsXml)
    {	m_qsIniFileName = pfd -> selectedFiles().at(0);
        SaveConfiguration(m_qsIniFileName);
    }
    setWindowTitle("QRmo-" + m_qsIniFileName);
    delete pfd;
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::addMessage(QString str) {
    console()->addMessage(str);
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::LoadConfiguration(QString qsFile) {
    QFile file(qsFile);
    QString qsErr;
    int i1, i2;

    if(file.open(QIODevice::ReadOnly))
    {	if(m_pDomProp -> setContent((QIODevice *)&file, &qsErr, &i1, &i2))
        {	file.close();
            initialize(m_pDomProp);
            // qDebug() << "QCPRmo::LoadConfiguration IniFile=" << qsFile;
        } else
        {	file.close();
            QString qs = "Read failed: file " + qsFile;
            qDebug() << "QCPRmo::LoadConfiguration " << qsErr.utf16() << " line, col=" << i1 << i2;
        }
    } else
    {	QString qs = "Open failed: file " + qsFile;
        qDebug() << qs;
    }
}
//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::initialize(QDomDocument * pDomProp) {
    QDomNode nd;
    QString qs;
    bool bOk;

    QDomNodeList ndl = pDomProp -> elementsByTagName("QRmo");
    if(ndl.count() < 1) return;
    QDomElement eRmo = ndl.at(0).toElement();

    ndl = eRmo.elementsByTagName("Geometry");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            qs = nd.toText().data();
            QByteArray baGeometry = QByteArray::fromHex((char *)qs.toAscii().data());
            restoreGeometry(baGeometry);
        }
    }
    ndl = eRmo.elementsByTagName("State");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            qs = nd.toText().data();
            QByteArray baGeometry = QByteArray::fromHex((char *)qs.toAscii().data());
            restoreState(baGeometry);
        }
    }
    ndl = eRmo.elementsByTagName("PropPageIndx");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            qs = nd.toText().data();
            m_PropPageIndx = qs.toInt();
        }
    }
    ndl = eRmo.elementsByTagName("ThisRmoId");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            qs = nd.toText().data();
            m_iRmoId = qs.toInt();
        }
    }
    ndl = eRmo.elementsByTagName("Port");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            qs = nd.toText().data();
            m_iPort = qs.toInt();
        }
    }
    ndl = eRmo.elementsByTagName("Address");
    if(ndl.count() > 0)
    {	nd = ndl.at(0);
        ndl = nd.childNodes();
        if(ndl.count() > 0)
        {	nd = ndl.at(0);
            m_Address = nd.toText().data();
        }
    }
    // invoke initialize() method by its name. the return value is of no interest
    QMetaObject::invokeMethod(this->localctrlqml(),"initialize",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));
    QMetaObject::invokeMethod(this->findicator(),"initialize",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));
    QMetaObject::invokeMethod(this->console(),"initialize",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));
}

//*****************************************************************************
//
//*****************************************************************************
void QCPRmo::getConfiguration(QDomDocument * pDomProp)
{	int i;
    bool bPiPresent = false;
    bool bComPresent = false;
    QDomElement eParam;
    QString qs;

    if(pDomProp == NULL) return;
    QDomNodeList ndl = pDomProp -> childNodes();
    i = 0;
    while(i < ndl.count())
    {	QDomNode nd = ndl.at(i);
        QDomNode::NodeType ndt = nd.nodeType();
#pragma GCC diagnostic ignored "-Wswitch"
        switch(ndt)
        {	case QDomNode::ProcessingInstructionNode:
                bPiPresent = true;
                break;

            case QDomNode::CommentNode:
                bComPresent = true;
                break;

            case QDomNode::ElementNode:
                if(nd.toElement().tagName() == "PARAMETERS")
                eParam = nd.toElement();
                break;
        }
#pragma GCC diagnostic pop
        i++;
    }
    if(!bPiPresent)
    {	QDomProcessingInstruction pi;
        pi = pDomProp -> createProcessingInstruction("xml", "version='1.0'");
        pDomProp -> appendChild(pi);
    }
    if(!bComPresent)
    {	QDomComment com;
        com = pDomProp -> createComment("Rmo parameters store file");
        pDomProp -> appendChild(com);
    }
    if(eParam.isNull())
    {	eParam = pDomProp -> createElement("PARAMETERS");
        pDomProp -> appendChild(eParam);
    }

    // invoke getConfiguration() method by its name. the return value is of no interest
    QMetaObject::invokeMethod(this->localctrlqml(),"getConfiguration",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));
    QMetaObject::invokeMethod(this->findicator(),"getConfiguration",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));
    QMetaObject::invokeMethod(this->console(),"getConfiguration",Qt::DirectConnection,Q_ARG(QDomDocument*,pDomProp));

    QDomNodeList imitl = eParam.elementsByTagName("QRmo");
    i = 0;
    while(i < imitl.count()) eParam.removeChild(imitl.at(i++));

    QDomElement eRmo = pDomProp -> createElement("QRmo");

    QDomElement e = pDomProp -> createElement("Geometry");
    QByteArray baGeometry = saveGeometry();
    qs = QString(baGeometry.toHex());
    QDomText t = pDomProp -> createTextNode(qs);
    e.appendChild(t);
    eRmo.appendChild(e);

    e = pDomProp -> createElement("State");
    baGeometry.clear();
    baGeometry = saveState();
    qs = QString(baGeometry.toHex());
    t = pDomProp -> createTextNode(qs);
    e.appendChild(t);
    eRmo.appendChild(e);

    e = pDomProp -> createElement("PropPageIndx");
    t = pDomProp -> createTextNode(qs.setNum(m_PropPageIndx));
    e.appendChild(t);
    eRmo.appendChild(e);

    e = pDomProp -> createElement("ThisRmoId");
    t = pDomProp -> createTextNode(qs.setNum(m_iRmoId));
    e.appendChild(t);
    eRmo.appendChild(e);

    e = pDomProp -> createElement("Port");
    t = pDomProp -> createTextNode(qs.setNum(m_iPort));
    e.appendChild(t);
    eRmo.appendChild(e);

    e = pDomProp -> createElement("Address");
    t = pDomProp -> createTextNode(m_Address);
    e.appendChild(t);
    eRmo.appendChild(e);
    eParam.appendChild(eRmo);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
