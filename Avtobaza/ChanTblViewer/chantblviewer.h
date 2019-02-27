#ifndef _chantblviewer_h_
#define _chantblviewer_h_

#include <QtCore>
#include <QtGui>
// needed for SIFLONGPAPAMS STROBLONGPARAMSHB STROBLONGPARAMSLB 

#ifdef __linux
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#else
    #include "qavtctrl.h"
    #include "codograms.h"
#endif

#include <ui_viewerdialog.h>
#include "stringlistmodel.h"

// use multiple inheritance to access form ViewerDialog (type: QMainWindow)
class ChanTblViewer :	public QMainWindow, 
						public Ui::ViewerDialog
{
Q_OBJECT
private:
	void openDatFile(QString fileName);
	QAction *actionSaveChanges;
	QAction *actionUndoChanges;
public:
	ChanTblViewer(QMainWindow* pmw = 0);
	QList<QCheckBox*> m_listSec;
	QList<QRadioButton*> m_listParam;

	StringListModel *m_model;


public slots:
    void slotReset();
    void slotOpen();
    void slotSave();
    void slotSaveCSV();
	void parameterSelected();
	void displayError(QString msg);
	void enableApply();
	void disableApply();

};

#endif  //_chantblviewer_h_

