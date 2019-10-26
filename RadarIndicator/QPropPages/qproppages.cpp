#include "qproppages.h"
#include <QPixmap>
#include <QVBoxLayout>
#include <QMetaObject>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QColorSelectionButton::QColorSelectionButton(const QColor &c, QWidget *parent /*=0*/) : 
  QPushButton(parent)
, m_qcColorSelection(c) 
, m_pcdColorDlg(NULL)  {
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
 : QDialog(parent,Qt::Dialog),
         m_pleDBFileName(NULL),
         m_pbAccept(NULL),
         m_pOwner (pOwner) {
	 resize(591,326);
     setWindowTitle("Properties");
     QVBoxLayout *pVLayout = new QVBoxLayout;
     QTabWidget *pTabWidget=new QTabWidget;
     pVLayout->addWidget(pTabWidget);
     QMetaObject::invokeMethod(m_pOwner,"fillTabs",Q_ARG(QObject *,this),Q_ARG(QObject *,pTabWidget));
     m_pbAccept = new QPushButton("Accept");
     pVLayout->addWidget(m_pbAccept);
     setLayout(pVLayout);
     QObject::connect(m_pbAccept,SIGNAL(clicked()),SLOT(onAccepted()));
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
    done(QDialog::Accepted);
}
