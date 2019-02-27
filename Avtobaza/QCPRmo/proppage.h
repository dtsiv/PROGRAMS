#ifndef QRMOPROPPAGE_H
#define QRMOPROPPAGE_H
#include "stdafx.h"
#include <QWidget>
#include <QColorDialog>
#include "ui_proppage.h"
class QCPRmo; 
//*****************************************************************************
//
//*****************************************************************************
class RmoPropPage : public QWidget, public Ui::QRmoPropPage
{
	Q_OBJECT

public:
	RmoPropPage(QWidget * parent, QCPRmo * iparent);
	~RmoPropPage()
	{
	//	DbgPrint(L"Rmo::PropPage DESTRUCTOR");
	}

public slots:
    void OnApplayChanges(PDWORD pEFl);

protected:

private:
	QCPRmo * m_pRmo;

public:
};
#endif // QRMOPROPPAGE_H


