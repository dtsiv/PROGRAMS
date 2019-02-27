#include "stdafx.h"
#include "qlocalcontrolqmlwidget.h"
#include "qlocalcontrolqmlwidget_p.h"
#include "proppage.h"
#include "programmodel.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//*****************************************************************************
// 
//*****************************************************************************
QLocalControlQMLWidget::QLocalControlQMLWidget(QWidget *parent)
        : QWidget(parent)
	    , d_ptr(new QLocalControlQMLWidgetPrivate()) {

	Q_D(QLocalControlQMLWidget);

	d->q_ptr = this;

	 // ProgramModel member
    d->m_pProgramModel = new ProgramModel;
	QObject::connect(d->m_pProgramModel,SIGNAL(uiParamsChanged()),SIGNAL(uiParamsChanged()));
	QObject::connect(d->m_pProgramModel,SIGNAL(SendCommand(char*,int,int)),SIGNAL(SendCommand(char*,int,int)));
	 // general QML setup: register types, set cppModel as QML context property
    d->m_pDeclarativeView = new QDeclarativeView;
	d->m_pDeclarativeView->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    QDeclarativeContext *ctxt = d->m_pDeclarativeView->rootContext();
    ctxt->setContextProperty("cppModel",d->m_pProgramModel);
	 // ui setup
	d->m_pLabelNoQml = new QLabel("<H1><CENTER>No *.qml!</CENTER></H1>");
	d->m_pVBoxLayout = new QVBoxLayout;
	d->m_pVBoxLayout->setContentsMargins(0,0,0,0);
	setLayout(d->m_pVBoxLayout);
	d->m_pathToQml=QString("view.qml");
	d->setQmlSource(d->m_pathToQml);
}

//*****************************************************************************
//
//*****************************************************************************
QLocalControlQMLWidget::~QLocalControlQMLWidget() {

	Q_D(QLocalControlQMLWidget);

    delete d->m_pLabelNoQml;
	delete d->m_pVBoxLayout;
    delete d->m_pProgramModel;
    delete d->m_pDeclarativeView;
}

//*****************************************************************************
//  this constructor enables inheritance for private classes
//*****************************************************************************
QLocalControlQMLWidget::QLocalControlQMLWidget(QLocalControlQMLWidgetPrivate /*must be &&*/ &dd,
											   QWidget *parent) 
        : QWidget(parent)
        , d_ptr(&dd) {

	Q_D(QLocalControlQMLWidget);

	d->q_ptr = this;
}

//*****************************************************************************
//
//*****************************************************************************
void QLocalControlQMLWidget::receiveLocalCtrlInfoEx(PLOCALCTRLSTRUCTEX pCtl,bool bAskForConfirmation/*=true*/) {

	Q_D(QLocalControlQMLWidget);

    d->lock();
    d->m_pProgramModel->receiveLocalCtrlInfoEx(pCtl,bAskForConfirmation);
    d->unlock();
}

//*****************************************************************************
//
//*****************************************************************************
void QLocalControlQMLWidget::receiveFrequenciesTable(PFREQUTBL pft,bool bAskForConfirmation/*=true*/) {

	Q_D(QLocalControlQMLWidget);

    d->lock();
    d->m_pProgramModel->receiveFrequenciesTable(pft,bAskForConfirmation);
    d->unlock();
}

//*****************************************************************************
//
//*****************************************************************************
void QLocalControlQMLWidget::initialize(QDomDocument * pDomProp) {

	Q_D(QLocalControlQMLWidget);

	QDomNode nd;
	QString qs;
	bool bOk;

	QDomNodeList ndl = pDomProp -> elementsByTagName("LocalControlQML");	
	if(ndl.count() < 1) return;
	QDomElement eLocalControlQML = ndl.at(0).toElement();

	ndl = eLocalControlQML.elementsByTagName("PathToQML");
	if(ndl.count() > 0)
	{	nd = ndl.at(0);	
		ndl = nd.childNodes();
		if(ndl.count() > 0)	
		{	nd = ndl.at(0);
			qs = nd.toText().data();
			setPathToQml(qs);
		}
	}

	for (int i=0; i<8; i++) {
		ndl = eLocalControlQML.elementsByTagName(QString("HotKey%1").arg(i));
		if(ndl.count() > 0)
		{	nd = ndl.at(0);	
			ndl = nd.childNodes();
			if(ndl.count() > 0)	
			{	nd = ndl.at(0);
				qs = nd.toText().data();
				setHotKey(i,qs);
			}
		}
	}

	//============ retrieve local control structure and frequency table and reset m_pProgramModel
	QString base64EncodedStruct;
	//== LOCALCTRLSTRUCTEX
	ndl = eLocalControlQML.elementsByTagName("APPLAYLOCALCTRLEX");	
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	ndl = nd.childNodes();
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	base64EncodedStruct=nd.toText().data();
	QByteArray baCtl = QByteArray::fromBase64(base64EncodedStruct.toLocal8Bit());
	//== FREQUTBL
	ndl = eLocalControlQML.elementsByTagName("FREQUTBL");	
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	ndl = nd.childNodes();
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	base64EncodedStruct=nd.toText().data();
	QByteArray baFreq = QByteArray::fromBase64(base64EncodedStruct.toLocal8Bit());
	//== sConventions
	ndl = eLocalControlQML.elementsByTagName("sConventions");	
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	ndl = nd.childNodes();
	if(ndl.count() != 1) return;
	nd = ndl.at(0);	
	QString sConventions = nd.toText().data();
	//== reset m_pProgramModel
	bool askForConfirmation=false;
	int iSize = sizeof(FREQUTBL) + (MAX_FREQ_IDX+1) * sizeof(DWORD);
	if (baFreq.size() != iSize || baCtl.size() != sizeof(APPLAYLOCALCTRLEX)) return; // can only keep whole structure
	//== protected section
    d->lock();
	d->m_pProgramModel->initConventions(sConventions);
	d->m_pProgramModel->initApplayLocalCtrlEx((PAPPLAYLOCALCTRLEX)baCtl.data());
	d->m_pProgramModel->initFrequencies((PFREQUTBL)baFreq.data());
	d->m_pProgramModel->afterLocalControlQMLWidgetInit();
    d->unlock();
	return;
}

//*****************************************************************************
//
//*****************************************************************************
void QLocalControlQMLWidget::getConfiguration(QDomDocument * pDomProp) {

	Q_D(QLocalControlQMLWidget);

	if(pDomProp == NULL) return;

	int i;
	bool bPiPresent = false;
	bool bComPresent = false;
	QDomElement eParam;
	QString qs;

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
		pi = pDomProp -> createProcessingInstruction("xml", "version='1.0' encoding='UTF-8");
		pDomProp -> appendChild(pi); 
	}
	if(!bComPresent)
	{	QDomComment com;
		com = pDomProp -> createComment("QRmo parameters store file");
		pDomProp -> appendChild(com); 
	}
	if(eParam.isNull())
	{	eParam = pDomProp -> createElement("PARAMETERS");
		pDomProp -> appendChild(eParam);
	}
	QDomNodeList nl = eParam.elementsByTagName("LocalControlQML");
	i = 0;
	while(i < nl.count()) eParam.removeChild(nl.at(i++));
	
	QDomElement eLocalControlQML = pDomProp -> createElement("LocalControlQML");
	QDomElement e;

	e = pDomProp -> createElement("PathToQML");
	QDomText t = pDomProp -> createTextNode(d->m_pathToQml);
	e.appendChild(t);
	eLocalControlQML.appendChild(e);

	for (i=0;i<8;i++) {
		QString tagName;
		tagName = QString("HotKey%1").arg(i);
		e = pDomProp -> createElement(tagName);
		QDomText t = pDomProp -> createTextNode(getHotKey(i));
		e.appendChild(t);
		eLocalControlQML.appendChild(e);
	}

	e = pDomProp -> createElement("APPLAYLOCALCTRLEX");
	t = pDomProp -> createTextNode(d->m_pProgramModel->ctlToBase64());
	e.appendChild(t);
	eLocalControlQML.appendChild(e);

	e = pDomProp -> createElement("FREQUTBL");
	t = pDomProp -> createTextNode(d->m_pProgramModel->ftblToBase64());
	e.appendChild(t);
	eLocalControlQML.appendChild(e);

	e = pDomProp -> createElement("sConventions");
	t = pDomProp -> createTextNode(d->m_pProgramModel->conventions());
	e.appendChild(t);
	eLocalControlQML.appendChild(e);

	eParam.appendChild(eLocalControlQML);
}

//*****************************************************************************
//
//*****************************************************************************
QStringList QLocalControlQMLWidget::about() {
	QStringList qsVer;
	QString qs;
	qs.sprintf("QLocalControlQMLWidgetLib v.%d.%02d beta by Tristan %04d",VER_MAJOR,VER_MINOR,VER_YEAR);
	qsVer << qs;
	return(qsVer);
}

//*****************************************************************************
//
//*****************************************************************************
QString QLocalControlQMLWidget::version() {
    QString qs;
    qs.sprintf("%d.%02d", VER_MAJOR, VER_MINOR);
    return(qs);
}

//*****************************************************************************
//
//*****************************************************************************
void QLocalControlQMLWidget::getPropPages(QList<QWidget *> * pwl, QWidget * pParent) {

	QWidget * pwg = (QWidget *)new PropPage(pParent,this);
	pwl -> append(pwg);
}

//================================================================================
//
//================================================================================
void QLocalControlQMLWidget::setHotKey(int i, QString hk) {

	Q_D(QLocalControlQMLWidget);

	if (i<0 || i>= d->m_pProgramModel->hotKeys.count()) return;
	d->m_pProgramModel->hotKeys.replace(i,hk);
}

//================================================================================
//
//================================================================================
QString QLocalControlQMLWidget::getHotKey(int i) {

	Q_D(QLocalControlQMLWidget);

	if (i<0 || i>= d->m_pProgramModel->hotKeys.count()) return QString();
	return d->m_pProgramModel->hotKeys.at(i);
}

//================================================================================
//
//================================================================================
void QLocalControlQMLWidget::setPathToQml(const QString &pathToQml) {

	Q_D(QLocalControlQMLWidget);

	d->m_pathToQml = pathToQml;
	d->setQmlSource(d->m_pathToQml);
}

//================================================================================
//
//================================================================================
QString QLocalControlQMLWidget::getPathToQml() {

	Q_D(QLocalControlQMLWidget);

	return d->m_pathToQml;
}

//*****************************************************************************
//
//*****************************************************************************
bool QLocalControlQMLWidgetPrivate::setQmlSource(const QString &localFile) {

    lock();
	QUrl viewSource = QUrl::fromLocalFile(localFile);
	if (!viewSource.isValid() || viewSource.isEmpty()) return false;

	if (!m_pVBoxLayout->isEmpty()) // previous widget exists
		// QMessageBox::information(0,"","Layout not empty!");
		while (m_pVBoxLayout->takeAt(0) != 0);
     // try to set DeclarativeView source
	if (QFile::exists(viewSource.toLocalFile())) m_pDeclarativeView->setSource(viewSource);
	if (m_pDeclarativeView->status() == QDeclarativeView::Ready) {
		m_pLabelNoQml->hide();
        // QML subsystem was reset by m_pDeclarativeView->setSource()
        // Need to initialize QML model and send necessary signals to QML engine
        m_pProgramModel->afterLocalControlQMLWidgetInit();
        m_pDeclarativeView->show();
        m_pVBoxLayout->addWidget(m_pDeclarativeView);
        // QML subsystem was reset by m_pDeclarativeView->setSource()
        // Need to initialize QML model and send necessary signals to QML engine
        // QTimer::singleShot(0,m_pProgramModel,SLOT(afterLocalControlQMLWidgetInit()));
	}
	else {
		m_pDeclarativeView->hide();
		m_pLabelNoQml->show();
		m_pVBoxLayout->addWidget(m_pLabelNoQml);
	}
    unlock();
    return true;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
