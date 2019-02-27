#include "stdafx.h"
#include "proppage.h"
#include "qcprmo.h"

//*****************************************************************************
//
//*****************************************************************************
RmoPropPage::RmoPropPage(QWidget * parent, QCPRmo * iparent) : QWidget(parent)
{
	m_pRmo = iparent;
	setupUi(this);
    lineEditAddress -> setText(m_pRmo ->m_Address );
	spinBoxPort -> setValue(m_pRmo -> m_iPort);
	spinBoxRmoId -> setValue(m_pRmo -> m_iRmoId);

}
//*****************************************************************************
//
//*****************************************************************************
void RmoPropPage::OnApplayChanges(PDWORD pEFl)
{
    // DbgPrint(L"RmoPropPage::OnApplayChanges errFl=%lx", *pEFl);

	bool bReconnect = m_pRmo -> m_Address != lineEditAddress -> text() || 
					  m_pRmo -> m_iPort != spinBoxPort -> value();
	m_pRmo -> m_Address = lineEditAddress -> text();
	m_pRmo -> m_iPort = spinBoxPort -> value();
	m_pRmo -> m_iRmoId = spinBoxRmoId -> value();
    if(bReconnect) m_pRmo->reconnect(m_pRmo -> m_Address,m_pRmo -> m_iPort);
}


