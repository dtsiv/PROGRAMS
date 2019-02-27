#ifndef RMOPROPDLG_H
#define RMOPROPDLG_H
#include "stdafx.h"
#include <QWidget>
#include <QDialog>
#include "ui_rmopropdlg.h"
class QCPRmo;
//*****************************************************************************
//
//*****************************************************************************
class RmoPropDlg : public QDialog, public Ui::QRmoPropDialog
{
	Q_OBJECT

public:
	RmoPropDlg(QWidget * parent = 0, Qt::WFlags flags = 0);
	~RmoPropDlg()
	{
		// DbgPrint(L"RmoPropDlg: DESTRUCTOR");
	}
	void AddPages(QList<QWidget *> * pwl);

public slots:
	void OnAccept(void);
	void OnDlgClose();

signals:
     void ApplayChanges(PDWORD pErrFlag);

protected:

private:
	QCPRmo * m_pRmo;

public:
};
#endif // RMOPROPDLG_H


