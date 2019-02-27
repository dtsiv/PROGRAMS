#include <QtGui>
#include "chantblviewer.h"

ChanTblViewer::ChanTblViewer(QMainWindow* pmw) : QMainWindow(pmw)
{
	// call Ui::ViewerDialog method to setup application form
	setupUi(this);

	m_path->setReadOnly(true);
	m_pte->setReadOnly(true);

	m_model = new StringListModel;

    // initialize the usual signal/slot and menu machinery
    connect(m_cmdReset, SIGNAL(clicked()), SLOT(slotReset()));
	// cleanup model cache when 'Update' button is clicked
	connect(m_cmdReset, SIGNAL(clicked()), m_model, SLOT(undoChanges()));

	menuFile->addAction("&Open", this, SLOT(slotOpen()));
	menuFile->addAction("&Update", this, SLOT(slotReset()));
	menuFile->addAction("&Save", this, SLOT(slotSave()));
	menuFile->addAction("Save as &CSV", this, SLOT(slotSaveCSV()));
	actionSaveChanges = menuFile->addAction("&Apply changes", m_model, SLOT(saveToDisk()));
	actionUndoChanges = menuFile->addAction("U&ndo changes", m_model, SLOT(undoChanges()), QKeySequence::Undo);
	menuFile->addSeparator();
	menuFile->addAction("&Exit", qApp, SLOT(quit()));

	// arrange checkboxes as a list
	m_listSec << m_g0 << m_g1 << m_g2 << m_g3 << m_g4 << m_g5 << m_g6 << m_g7; 

	// arrange radiobuttons as a list
	m_listParam.insert(PAR_bAttSif,radioButton_bAttSif);
	m_listParam.insert(PAR_sTrashSif,radioButton_sTrashSif);
	m_listParam.insert(PAR_sTrashSif1,radioButton_sTrashSif1);
	m_listParam.insert(PAR_bSifOutMult,radioButton_bSifOutMult);
	m_listParam.insert(PAR_bAttWbr,radioButton_bAttWbr);
	m_listParam.insert(PAR_bAtt,radioButton_bAtt);
	m_listParam.insert(PAR_sTrash,radioButton_sTrash);
	m_listParam.insert(PAR_sTrash1,radioButton_sTrash1);
	m_listParam.insert(PAR_bWbrOutMult,radioButton_bWbrOutMult);
	m_listParam.insert(PAR_bAttL,radioButton_bAttL);
	m_listParam.insert(PAR_sTrashLbr,radioButton_sTrashLbr);
	m_listParam.insert(PAR_sTrashLbr1,radioButton_sTrashLbr1);
	m_listParam.insert(PAR_bLbrOutMult,radioButton_bLbrOutMult);
	m_listParam.insert(PAR_bAttWba,radioButton_bAttWba);
	m_listParam.insert(PAR_sTrashWba,radioButton_sTrashWba);
	m_listParam.insert(PAR_sTrashWba1,radioButton_sTrashWba1);
	m_listParam.insert(PAR_bWbaOutMult,radioButton_bWbaOutMult);

	foreach (QRadioButton *rb,m_listParam) {
		QObject::connect(rb,SIGNAL(clicked()),SLOT(parameterSelected()));
	}

	QObject::connect(spinSel,SIGNAL(valueChanged(int)),m_model,SLOT(setSel(int)));
	QObject::connect(spinSec,SIGNAL(valueChanged(int)),m_model,SLOT(setSec(int)));
	QObject::connect(spinFreq,SIGNAL(valueChanged(int)),m_model,SLOT(setFreq(int)));
	
	// update plainTextEdit automatically on tab currentChanged signal
	QObject::connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(slotReset()));

	// enable error messages via m_pte
	QObject::connect(m_model,SIGNAL(reportError(QString)),this,SLOT(displayError(QString)));

	// enable Apply button on data change
	QObject::connect(m_model,SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),this,SLOT(enableApply()));

	// disable Apply button on save
	QObject::connect(m_model,SIGNAL(changesSaved()),this,SLOT(disableApply()));

	// disable Apply button on undo
	QObject::connect(m_model,SIGNAL(changesReverted()),this,SLOT(disableApply()));

	QObject::connect(applyButton,SIGNAL(clicked()),actionSaveChanges,SLOT(trigger()));
	// try open dat file in CWD
	QDir cwd("."); 
	openDatFile(QDir::cleanPath(cwd.absolutePath()+QString("/uvsctrltbl.dat")));

	disableApply();

	return;
}

// parameter was selected
void ChanTblViewer::parameterSelected() {
	int iParam = m_listParam.indexOf((QRadioButton*)sender());

	if (iParam == -1) return;

	if ( iParam == PAR_bAttSif	 ||
		 iParam == PAR_sTrashSif	 ||
		 iParam == PAR_sTrashSif1 ||
		 iParam == PAR_bSifOutMult ) 
		spinFreq->setEnabled(false);
	else
		spinFreq->setEnabled(true);

	m_model->setParam(iParam);
}

// click on Update button or select mene File|Update
void ChanTblViewer::slotReset()
{
	int iSec, iFreq;

	QString str("");
	
	if (!m_model->isInitialized()) return;

	//==========================================================

	bool bsif = m_sif->isChecked();
	bool bwbr = m_wbr->isChecked();
	bool blbr = m_lbr->isChecked();
	bool bwba = m_wba->isChecked();

	int iSel = qBound(0, m_iatb->text().toInt(), 31);
	int iffrom = qBound(0, m_ffrom->text().toInt(), 2047);
	int ifto = qBound(iffrom, m_fto->text().toInt(), 2047);

    //==============  sif sections ===============
	if (bsif) {
		for (iSec=0; iSec < m_listSec.size(); iSec++) {
			if (m_listSec[iSec]->isChecked()) {
				str += QString("[sif%1]\n").arg(iSec);
				str += m_model->makeSifString(iSel, iSec);
				str += QString("\n");
			}
		}
	}
    //==============  wbr sections ===============
	if (bwbr) {
		for (iSec=0; iSec<m_listSec.size(); iSec++) {
			if (m_listSec[iSec]->isChecked()) {
				str += QString("[wbr%1]\n").arg(iSec);
				for (iFreq=iffrom; iFreq<=ifto; iFreq++)
					str += m_model->makeWbrString(iSel, iSec, iFreq);
				str += QString("\n");
			}
		}
	}
    //==============  lbr sections ===============
	if (blbr) {
		for (iSec=0; iSec<m_listSec.size(); iSec++) {
			if (m_listSec[iSec]->isChecked()) {
				str += QString("[lbr%1]\n").arg(iSec);
				for (iFreq=iffrom; iFreq<=ifto; iFreq++)
					str += m_model->makeLbrString(iSel, iSec, iFreq);
				str += QString("\n");
			}
		}
	}
    //==============  wba sections ===============
	if (bwba) {
		for (iSec=0; iSec<m_listSec.size(); iSec++) {
			if (m_listSec[iSec]->isChecked()) {
				str += QString("[wba%1]\n").arg(iSec);
				for (iFreq=iffrom; iFreq<=ifto; iFreq++)
					str += m_model->makeWbaString(iSel, iSec, iFreq);
				str += QString("\n");
			}
		}
	}
	//========== update PlainTextEdit =============
	m_pte->setPlainText(str);
}

// Simply save the contents of QPlainTextEdit into *.atb file of user's choice
void ChanTblViewer::slotSave() {
	int iAttTblIdx = 0;
	QStringList lst;
	QString str;

	QDir cwd("."); 
	QString atbFileName(cwd.absolutePath()+QString("/tbl%1").arg(iAttTblIdx));

	QString m_qsFileName = QFileDialog::getSaveFileName(this, tr("Save as attenuation table (atb) file"),
                            atbFileName, tr("Attenuation Table Files (*.atb)"));
	
	QFile m_fAttTblFile(m_qsFileName);
	if(!m_fAttTblFile.open(QIODevice::WriteOnly)) 
	{	str = QString("Failed to open attenuation table file %1 for writing").arg(m_qsFileName);
		m_pte->setPlainText(str);
		return;
	}
	QTextStream qtsOut(&m_fAttTblFile);
    qtsOut << m_pte->toPlainText() << endl;
	m_fAttTblFile.close();

	return;
}

// open binary uvsctrltbl.dat of user's choice and refresh the QPlainTextEdit
void ChanTblViewer::slotOpen() {
	int iAttTblIdx = 0;
	QStringList lst;
	QString str;

	QDir cwd("."); 
	QString atbFileName(cwd.absolutePath()+QString("/uvsctrltbl.dat"));	

	QString fileName = QFileDialog::getOpenFileName(this, tr("Binary (dat) file"),
                            atbFileName, tr("Binary files (*.dat)"));

	openDatFile(fileName);

	return;
}

// call m_model->openDatFile(fileName) and update interface
void ChanTblViewer::openDatFile(QString fileName) {

	tabWidget->setTabEnabled(1,false);

	m_model->openDatFile(fileName);

	// check the result
	if (m_model->isInitialized()) {

		// if the model is initialized set it to the view
		listVal->setModel(m_model);
	
		// fill path to file
		m_path->setText(QDir::cleanPath(fileName));

		// get parameter index from model
		int iParam = m_model->param();

		// enable spinFreq or disable spinFreq for Sif parameters
		if ( iParam == PAR_bAttSif	 ||
			 iParam == PAR_sTrashSif	 ||
			 iParam == PAR_sTrashSif1 ||
			 iParam == PAR_bSifOutMult ) 
			spinFreq->setEnabled(false);
		else
			spinFreq->setEnabled(true);

        // check radio button for parameter m_model->param()
		m_listParam[qBound(0,iParam,m_listParam.size()-1)]->setChecked(true);

		// update the plain text edit
		slotReset();

		tabWidget->setTabEnabled(1,true);

	}
}

// save all data from the currently selected uvsctrltbl.dat file to CSV
// saparator - "," - be careful with Excel!!!
void ChanTblViewer::slotSaveCSV() {
	int iAttTblIdx = 0;
	QStringList lst;
	int i, iSel, iSec, iFreq;

	QString str("");
	
	if (!m_model->isInitialized()) return;
	//==========================================================

	int iffrom = qBound(0, m_ffrom->text().toInt(), 2047);
	int ifto = qBound(iffrom, m_fto->text().toInt(), 2047);


	QDir cwd("."); 
	QString atbFileName(cwd.absolutePath()+QString("/tbl%1").arg(iAttTblIdx));

	QString m_qsCSVFileName = QFileDialog::getSaveFileName(this, tr("Save as CSV file"),
                            atbFileName, tr("CSV Files (*.csv)"));
	
	QFile m_fCSVFile(m_qsCSVFileName);
	
	if(!m_fCSVFile.open(QIODevice::WriteOnly)) 
	{	str = QString("Failed to open file %1 for writing").arg(m_qsCSVFileName);
		m_pte->setPlainText(str);
		return;
	}
	QTextStream qtsOut(&m_fCSVFile);

	for (iSel=0; iSel<ARRAY_SIZE_SEL; iSel++) {
		for (iSec=0; iSec<ARRAY_SIZE_SEC; iSec++) {
			for (iFreq=iffrom; iFreq<=ifto; iFreq++) {
				qtsOut << iSel << "," << iSec << ","  << iFreq << ",";
				for (i=0; i<2; i++) {
					int bAttWbr_local = m_model->bAttWbr(iSel,iSec,iFreq,i);
					qtsOut << bAttWbr_local;
					qtsOut << ",";
				}
				for (i=0; i<14; i++) {
					int bAtt_local = m_model->bAtt(iSel,iSec,iFreq,i);
					qtsOut << bAtt_local;
					qtsOut << ",";
				}
				for (i=0; i<14; i++) {
					int sTrash_local = m_model->sTrash(iSel,iSec,iFreq,i);
					qtsOut << sTrash_local;
					if (i<13) qtsOut << ",";
				}
				qtsOut << endl;
			}
		}
	}
	m_fCSVFile.close();

	return;
}

void ChanTblViewer::displayError(QString msg) {
	m_pte->setPlainText(msg);
	return;
}

void ChanTblViewer::enableApply() {
	applyButton->setEnabled(true);
	actionSaveChanges->setEnabled(true);
	actionUndoChanges->setEnabled(true);
	return;
}

void ChanTblViewer::disableApply() {
	applyButton->setEnabled(false);
	actionSaveChanges->setEnabled(false);
	actionUndoChanges->setEnabled(false);
	return;
}
