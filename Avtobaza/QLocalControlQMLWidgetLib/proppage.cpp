#include "stdafx.h"
#include <QObject>
#include "proppage.h"

PropPage::PropPage(QWidget *parent, QLocalControlQMLWidget * iparent)
	: QWidget(parent)
{
	setupUi(this);
	
	m_pLocalControlQMLWidget = iparent;
	lineEditPrg0->setText(m_pLocalControlQMLWidget->getHotKey(0));
	lineEditPrg1->setText(m_pLocalControlQMLWidget->getHotKey(1));
	lineEditPrg2->setText(m_pLocalControlQMLWidget->getHotKey(2));
	lineEditPrg3->setText(m_pLocalControlQMLWidget->getHotKey(3));
	lineEditPrg4->setText(m_pLocalControlQMLWidget->getHotKey(4));
	lineEditPrg5->setText(m_pLocalControlQMLWidget->getHotKey(5));
	lineEditPrg6->setText(m_pLocalControlQMLWidget->getHotKey(6));
	lineEditPrg7->setText(m_pLocalControlQMLWidget->getHotKey(7));
	lineEditQmlFile->setText(m_pLocalControlQMLWidget->getPathToQml());

    QObject::connect(toolButtonSetQmlFile,SIGNAL(clicked()),this,SLOT(OnPathToQmlButtonClicked()));
}

void PropPage::OnApplayChanges(PDWORD pEFl) {
	//DbgPrint(L"LocalControlQMLWidget PropPage::OnApplayChanges errFl=%lx", *pEFl);

	m_pLocalControlQMLWidget->setHotKey(0,lineEditPrg0->text());
	m_pLocalControlQMLWidget->setHotKey(1,lineEditPrg1->text());
	m_pLocalControlQMLWidget->setHotKey(2,lineEditPrg2->text());
	m_pLocalControlQMLWidget->setHotKey(3,lineEditPrg3->text());
	m_pLocalControlQMLWidget->setHotKey(4,lineEditPrg4->text());
	m_pLocalControlQMLWidget->setHotKey(5,lineEditPrg5->text());
	m_pLocalControlQMLWidget->setHotKey(6,lineEditPrg6->text());
	m_pLocalControlQMLWidget->setHotKey(7,lineEditPrg7->text());
	m_pLocalControlQMLWidget->setPathToQml(lineEditQmlFile->text());
}

void PropPage::OnPathToQmlButtonClicked(void) {
	QFileDialog * pfd;

	pfd = new QFileDialog(this, 
		QString((QChar *)L"Use QML interface description file ..."),
		QString(),
		"*.qml");
	pfd -> setFileMode(QFileDialog::ExistingFile);
	pfd -> setViewMode(QFileDialog::Detail);
	pfd -> selectFile(lineEditQmlFile -> text());

	if(!pfd -> exec())
	{	delete pfd;
		return;
	}
	const ushort * buf = QDir::current().absolutePath().utf16();
	// GetCurrentDirectory(512, buf);
	QDir curdir(QString((QChar *)buf));
	QString qsd = curdir.relativeFilePath(pfd -> selectedFiles().at(0));
	if(qsd.isNull()) qsd = ".";
	lineEditQmlFile -> setText(qsd);
	delete pfd;
}


PropPage::~PropPage()
{

}
