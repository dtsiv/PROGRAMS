#include "stdafx.h"
#include "proppagedlg.h"
#include "qcprmo.h"
//*****************************************************************************
//
//*****************************************************************************
RmoPropDlg::RmoPropDlg(QWidget * parent, Qt::WFlags flags) : QDialog(parent, flags)
{
	m_pRmo = (QCPRmo *)parent; 
	setupUi(this);
}
//*****************************************************************************
//
//*****************************************************************************
void RmoPropDlg::AddPages(QList<QWidget *> * pwl)
{	int i = 0;

	while(i < pwl -> count())
	{	QWidget * pwg = pwl -> at(i);
		tabWidgetPropPages -> addTab(pwg, pwg -> windowTitle());
		i++;
	}
	i = pwl -> count() - 1;
	while(i >= 0)
	{	QWidget * pwg = pwl -> at(i);
        QObject::connect(
            this,
            SIGNAL(ApplayChanges(PDWORD)),
            pwg,
            SLOT(OnApplayChanges(PDWORD)),
            Qt::DirectConnection);
        i--;
	}
	tabWidgetPropPages -> setCurrentIndex(m_pRmo -> m_PropPageIndx);
}
//*****************************************************************************
//
//*****************************************************************************
void RmoPropDlg::OnAccept(void)
{	DWORD dwFlag = 0;

	emit ApplayChanges(&dwFlag);
    if(dwFlag & 0xffff) return;
    done(dwFlag);
}
//*****************************************************************************
//
//*****************************************************************************
void RmoPropDlg::OnDlgClose()
{
    m_pRmo -> m_PropPageIndx = tabWidgetPropPages -> currentIndex();
// 	DbgPrint(L"RmoPropDlg::OnDlgClose");
}
