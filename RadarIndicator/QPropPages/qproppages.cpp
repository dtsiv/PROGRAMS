#include "qproppages.h"
#include "qinisettings.h"

#include <QPixmap>
#include <QVBoxLayout>
#include <QMetaObject>


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColorSelectionButton::QColorSelectionButton(const QColor &c, QWidget *parent /*=0*/) : 
  QPushButton(parent)
        , m_qcColorSelection(c)
        , m_pcdColorDlg(NULL) {
	setContentsMargins(2,2,2,2);
	QObject::connect(this,SIGNAL(clicked()),SLOT(onClicked()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColorSelectionButton::~QColorSelectionButton() {
	if (m_pcdColorDlg) delete m_pcdColorDlg;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColor& QColorSelectionButton::getSelection() {
	return m_qcColorSelection;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::paintEvent(QPaintEvent *pe) {
	this->QPushButton::paintEvent(pe);
	QPainter painter(this);
	painter.fillRect(contentsRect(), m_qcColorSelection);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::onClicked() {
	m_pcdColorDlg = new QColorDialog (m_qcColorSelection);
	m_pcdColorDlg->setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));
    m_pcdColorDlg->setOption(QColorDialog::ShowAlphaChannel);
	m_pcdColorDlg->open(this, SLOT(onColorSelected(QColor)));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QColorSelectionButton::onColorSelected(const QColor &color) {
	m_qcColorSelection=color;
	update();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPropPages::QPropPages(QObject *pOwner, QWidget *parent /* =0 */)
 : QDialog(parent,Qt::Dialog)
         , m_pOwner(pOwner)
         , m_pleDBFileName(NULL)
         , m_pleRegFileName(NULL)
         , m_ppbAccept(NULL)
         , m_ppbParse(NULL)
         , m_pleFCarrier(NULL)
         , m_pleTSampl(NULL)
         , m_pcbParseWholeDir(NULL)
         , m_pcbAdaptiveGrid(NULL)
         , m_ppbarParseProgress(NULL)
         , m_pleTimerMSecs(NULL)
         , m_pleNoiseMapFName(NULL)
         , m_pcbUseNoiseMap(NULL)
         , m_ppbarGenerateNoiseMap(NULL)
         , m_ppbGenerateNoiseMap(NULL) {

	 resize(591,326);
     setWindowTitle("Properties");
     setWindowIcon(QIcon(QPixmap(":/Resources/settings.ico")));
     QVBoxLayout *pVLayout = new QVBoxLayout;
     m_pTabWidget=new QTabWidget;
     m_pTabWidget->setStyleSheet(
       "QTabBar::tab:selected { background: lightgray; } ");

     pVLayout->addWidget(m_pTabWidget);
     QMetaObject::invokeMethod(m_pOwner,"fillTabs",Q_ARG(QObject *,this),Q_ARG(QObject *,m_pTabWidget));
     QPushButton *pbCancel = new QPushButton("Cancel");
     m_ppbAccept = new QPushButton("Accept");
     m_ppbAccept->setDefault(true);
     QHBoxLayout *pHLayout = new QHBoxLayout;
     pHLayout->addStretch();
     pHLayout->addWidget(pbCancel);
     pHLayout->addSpacing(20);
     pHLayout->addWidget(m_ppbAccept);
     pVLayout->addLayout(pHLayout);
     setLayout(pVLayout);
     QObject::connect(m_ppbAccept,SIGNAL(clicked(bool)),SLOT(onAccepted()));
     QObject::connect(pbCancel,SIGNAL(clicked(bool)),SLOT(close()));
     QObject::connect(this,SIGNAL(doParse()),m_pOwner,SLOT(onParseDataFile()));
     QIniSettings &iniSettings = QIniSettings::getInstance();
     QIniSettings::STATUS_CODES scStat;
     bool bOk=false;
     iniSettings.setDefault(QPROPPAGES_ACTIVE_TAB,-1);
     int iActiveTab = iniSettings.value(QPROPPAGES_ACTIVE_TAB,scStat).toInt(&bOk);
     if (bOk && iActiveTab >= 0 && iActiveTab < m_pTabWidget->count()) m_pTabWidget->setCurrentIndex(iActiveTab);

     // connect QPropPages::signals and QIndicatorWindow::slots
     QObject::connect(this,SIGNAL(updateParseProgressBar(double)),m_pOwner,SLOT(onUpdateParseProgressBar(double)));
     QObject::connect(this,SIGNAL(updateGenerateNoiseMapProgressBar(double)),m_pOwner,SLOT(onUpdateGenerateNoiseMapProgressBar(double)));
     QObject::connect(this,SIGNAL(generateNoiseMapFile()),m_pOwner,SLOT(onNoiseMapFileGenerate()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QPropPages::~QPropPages() {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QPropPages::onAccepted() {
    QMetaObject::invokeMethod(m_pOwner,"propChanged",Q_ARG(QObject *,this));
    if (m_pTabWidget) {
        QIniSettings &iniSettings = QIniSettings::getInstance();
        iniSettings.setValue(QPROPPAGES_ACTIVE_TAB,m_pTabWidget->currentIndex());
    }
    done(QDialog::Accepted);
}
