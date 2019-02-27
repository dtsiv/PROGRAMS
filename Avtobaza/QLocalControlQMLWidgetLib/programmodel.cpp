#include "stdafx.h"

#include "programmodel.h"
#include "targetselection.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068)
#endif

//================================================================================
//
//================================================================================
ProgramModel::ProgramModel(QObject *object /*=0*/) : QObject(object) {
	 // in case the hot keys were not loaded from disk
	hotKeys << "Ctrl+1" << "Ctrl+2" << "Ctrl+3" << "Ctrl+4" << "Ctrl+5" << "Ctrl+6" << "Ctrl+7" << "Ctrl+8";
	m_pACtl = (PAPPLAYLOCALCTRLEX)new APPLAYLOCALCTRLEX;
	m_pCtrl = (PLOCALCTRLSTRUCTEX)new LOCALCTRLSTRUCTEX;
	m_pMainCtrl = (PMAINCTRL)new MAINCTRL;
	// PFREQUTBL m_pFtbl memory allocation
	int iStart=0, iCou=MAX_FREQ_IDX+1, iSize;
	iSize = sizeof(FREQUTBL) + iCou*sizeof(DWORD);
	m_pFtbl = (PFREQUTBL)new char[iSize];
	m_pFtbl->iStartIndx=iStart;
	m_pFtbl->iCount=iCou;

    // change flags for frequencies table
    m_pFreqChng = (bool *)new bool[MAX_FREQ_IDX+1];
    // assume no change if not initialized
    for (int i=0; i<=MAX_FREQ_IDX; i++) m_pFreqChng[i]=false;

	// signal connections
    QObject::connect(
        this,
        SIGNAL(localCtrlInfoExReceived(char *, bool)),
        SLOT(onLocalCtrlInfoExReceived(char *, bool)),
        Qt::DirectConnection);
    QObject::connect(
        this,
        SIGNAL(frequenciesReceived(char *, bool)),
        SLOT(onFrequenciesReceived(char *, bool)),
        Qt::DirectConnection);
}

//================================================================================
//
//================================================================================
ProgramModel::~ProgramModel() {
    lock();
    delete[] (bool *)m_pFreqChng;
	delete[] (char *)m_pFtbl;
	delete m_pMainCtrl;
	delete m_pCtrl;
	delete m_pACtl;
    unlock();
}

//================================================================================
//
//================================================================================
void ProgramModel::setConventions(QString str) {
	if (str != m_qsConventions) {
        // lock(); This function is only called from within onLocalCtrlInfoExReceived()
        //         function, which is already protected by lock()
		m_qsConventions = str;
		emit conventionsChanged();
        // unlock();
	}
}

//================================================================================
//
//================================================================================
void ProgramModel::initConventions(QString str) {
	m_qsConventions = str;
}

//================================================================================
//
//================================================================================
void ProgramModel::initApplayLocalCtrlEx(PAPPLAYLOCALCTRLEX pAlc) {
    lock();
	memcpy(m_pACtl, pAlc, sizeof(APPLAYLOCALCTRLEX));
    unlock();
}

//================================================================================
//
//================================================================================
void ProgramModel::initFrequencies(PFREQUTBL pft) {
	int iStart, iCou, iSize;
	getFrequTblSize((PFREQUTBL)pft, iStart, iCou, iSize);
    lock();
	memcpy(&m_pFtbl->freq[iStart], &((PFREQUTBL)pft)->freq[0], iCou*sizeof(DWORD));

    // assume the value restored from disk as new value
    for (int i=0; i<iCou; i++) m_pFreqChng[iStart+i]=true;
    unlock();
}

//================================================================================
//
//================================================================================
void ProgramModel::afterLocalControlQMLWidgetInit() {
    emit conventionsChanged();
    emit paramsChanged();
    // emit refreshControls();  // this is called by onParamsChanged
}

//================================================================================
//
//================================================================================
QString ProgramModel::conventions() {
	return m_qsConventions;
}

//================================================================================
//
//================================================================================
int ProgramModel::matchShortCut(int key, int modifiers) {
	// DbgPrint(L"You pressed: %s", QKeySequence(key+modifiers).toString().utf16());
	for (int i=0; i<hotKeys.size(); ++i) {
		// DbgPrint(L"at %i: %s", i, QKeySequence(hotKeys.at(i)).toString().utf16());
		if (QKeySequence(key+modifiers).matches(QKeySequence(hotKeys.at(i)))==QKeySequence::ExactMatch) return i;
	}
	return -1;
}

//================================================================================
//
//================================================================================
quint32 ProgramModel::getRegisterValue(quint32 idx, quint32 mask, quint32 shift) {
	if (idx>MAX_REG_IDX || shift>MAX_SHIFT) return 0xFFFFFFFF; // invalid register value
	quint32 dwRegister;
	if (idx==REG_IDX_CND) // register CND
		dwRegister=m_pACtl->dwConditionFlags;
	else                  // 0 < idx < 7
		dwRegister=m_pACtl->dwRegisters[idx];
    // apply masks
	dwRegister &= mask;
	dwRegister >>= shift;
	// DbgPrint(L"In getRegisterValue: idx, mask, shift, value=%lx %lx %lx %lx",
	//          idx, mask, shift, dwRegister);
	return (dwRegister);
}

//================================================================================
//
//================================================================================
void ProgramModel::setRegisterValue(quint32 idx, quint32 value, quint32 mask, quint32 shift) {
	// idx - index of register according to qavtlocalcontrollib.cpp
	// idx = 0: A; 1: B; 2: C; 3: D; 4: E; 5: F; 6: Fh (same as FD high); 7: used HERE as CND; 
	quint32 oldValue;
	if (idx>MAX_REG_IDX || shift>MAX_SHIFT) return;
	// get old value
    lock();
	if (idx==REG_IDX_CND) // register CND
		oldValue=m_pACtl->dwConditionFlags;
	else                  // 0 < idx < 7
		oldValue=m_pACtl->dwRegisters[idx];
    // apply masks
	value <<= shift;
	value &= mask;
	// check if something has changed. If not - quit
	if ((oldValue&mask)==value) { unlock(); return; }
	// construct new value from unchanged and changed bits 
	oldValue &= ~mask;
	value |= oldValue;
	// set new values in the m_pACtl fields
	if (idx==REG_IDX_CND) // register CND
		m_pACtl->dwConditionFlags = value;
	else
		m_pACtl->dwRegisters[idx] = value;
	// Change mask -- see codograms.h, struct APPLAYLOCALCTRLEX_ // 0x1 Register 0 - valid, ... 0x10 Reg 4 - valid,
	// m_pACtl->dwChangeMask			                         // 0x100 - prog indx - valid, 0x200 cnd flags - valid
	if (idx==REG_IDX_CND)
		m_pACtl->dwChangeMask |= 0x200;
	else
		m_pACtl->dwChangeMask |= (1 << idx);
    unlock();
	//DbgPrint(L"In setRegisterValue: idx, value, mask, shift, oldValue | value=%lx %lx %lx %lx %lx",
	//           idx, value, mask, shift, oldValue | value);
}

//================================================================================
//
//================================================================================
quint32 ProgramModel::getFrequencyValue(quint32 idx, quint32 mask, quint32 shift) {
	if (idx>MAX_FREQ_IDX || shift>MAX_SHIFT) return 0xFFFFFFFF;
	quint32 dwFrequency=m_pFtbl->freq[idx].dwFrequ;
	dwFrequency &= mask;
	// DbgPrint(L"In getFrequencyValue: idx, mask, shift, dwFrequency, dwFrequency>>sh=%lu %lu %lu %lu %lu",
	// 	idx,mask,shift,dwFrequency,dwFrequency >> shift);
	return (dwFrequency >> shift);
}

//================================================================================
//
//================================================================================
void ProgramModel::setFrequencyValue(quint32 idx, quint32 value, quint32 mask, quint32 shift) {
	if (idx>MAX_FREQ_IDX || shift>MAX_SHIFT) return;
    lock();
	quint32 oldValue=m_pFtbl->freq[idx].dwFrequ;
	value <<= shift;
	value &= mask;
#pragma GCC diagnostic ignored "-Wparentheses"
    quint32 newValue = oldValue & ~mask | value;
#pragma GCC diagnostic pop
    if (newValue != oldValue) {
        m_pFtbl->freq[idx].dwFrequ = newValue;
        // set frequency change flag
        m_pFreqChng[idx]=true;
    }
    unlock();
	// DbgPrint(L"In setFrequencyValue: idx, value, mask, shift, oldValue | value=%lx %lx %lx %lx %lx",
	//		idx, value, mask, shift, oldValue | value);
}

//================================================================================
//
//================================================================================
QString ProgramModel::ftblToBase64() {
	int iStart, iCou, iSize;
	getFrequTblSize(m_pFtbl, iStart, iCou, iSize);
	QByteArray ba = QByteArray::fromRawData((char *)m_pFtbl,iSize);
	//DbgPrint(L"ftblToBase64: freq[0]=%lx",m_pFtbl -> freq[0]);
	return ba.toBase64();
}

//================================================================================
//
//================================================================================
QString ProgramModel::ctlToBase64() {
	QByteArray ba = QByteArray::fromRawData((char *)m_pACtl, sizeof(APPLAYLOCALCTRLEX));
	return ba.toBase64();
}

//================================================================================
//
//================================================================================
void ProgramModel::receiveFrequenciesTable(PFREQUTBL pft, bool askForConfirmation/*=true*/) {
    emit frequenciesReceived((char *)pft, askForConfirmation);
}

//================================================================================
//
//================================================================================
void ProgramModel::onFrequenciesReceived(char * pft, bool askForConfirmation) {
	int response;
	if (askForConfirmation)	
		response=QMessageBox::question(0,tr("Control panel QML"),
					 tr("Received FREQUTBL data. Apply?"),
					 QMessageBox::Ok,QMessageBox::Cancel);
	if (!askForConfirmation ||  response==QMessageBox::Ok) {
        lock();
		int iStart, iCou, iSize;
		getFrequTblSize((PFREQUTBL)pft, iStart, iCou, iSize);
		memcpy(&m_pFtbl->freq[iStart], &((PFREQUTBL)pft)->freq[0], iCou*sizeof(DWORD));

        // clear the frequencies change flags
        for (int i=0; i<iCou; i++) {
            m_pFreqChng[iStart+i]=false;
        }

		emit paramsChanged();
        unlock();
	}
}

//================================================================================
//
//================================================================================
void ProgramModel::getFrequTblSize(PFREQUTBL pFtblSrc, int &iStart, int &iCou, int &iSize) {
	iStart = max(0, min(pFtblSrc->iStartIndx, MAX_FREQ_IDX));
	iCou   = max(0, min(pFtblSrc->iCount,    (MAX_FREQ_IDX+1) - iStart));
	iSize  = sizeof(FREQUTBL) + iCou * sizeof(DWORD);
}

//================================================================================
//
//================================================================================
void ProgramModel::receiveLocalCtrlInfoEx(PLOCALCTRLSTRUCTEX plci, bool askForConfirmation/*=true*/) {
	emit localCtrlInfoExReceived((char *)plci,askForConfirmation);
}

//================================================================================
//
//================================================================================
void ProgramModel::onLocalCtrlInfoExReceived(char * buf, bool askForConfirmation) {
        PLOCALCTRLSTRUCTEX pCtrl = (PLOCALCTRLSTRUCTEX) buf;
	int response;
        int iSelected = pCtrl -> iSelected;

	QString mesg=QString("Received LOCALCTRLSTRUCTEX data.\nConventions: %1. iSelected: %2. Modename: %3\n Apply in control panel or cancel?")
           .arg(QString::fromLocal8Bit((const char *)&pCtrl->mode[iSelected].sConventions[0]))
           .arg(iSelected)
           .arg(QString::fromUtf16((const ushort *)&pCtrl->mode[iSelected].sModeName[0]));
	if (askForConfirmation)	
		response=QMessageBox::question(0,"Control panel QML",mesg,
					QMessageBox::Ok,QMessageBox::Cancel);
	if (!askForConfirmation || response == QMessageBox::Ok) {
        lock();
		// first, copy LOCALCTRLSTRUCTEX as a whole
		memcpy(m_pCtrl, pCtrl, sizeof(LOCALCTRLSTRUCTEX));
		// obtain sConventions[] for the currently selected mode m_pCtrl->iSelected
		setConventions(QString((char *)&m_pCtrl->mode[m_pCtrl->iSelected].sConventions[0]));
		// fill APPLAYLOCALCTRLEX structure with the received values (*m_pCtrl)
		m_pACtl->dwChangeMask=0;
		m_pACtl->dwConditionFlags=0;
		m_pACtl->iInternalIndx=m_pCtrl->iSelected;
		memcpy((char *)&m_pACtl->dwRegisters[0],(char *)&m_pCtrl->dwRegisters[0],sizeof(m_pACtl->dwRegisters));
        unlock();
        emit paramsChanged();
	}
}

//================================================================================
//
//================================================================================
void ProgramModel::submitLocalCtrlEx() {	
	// DbgPrint(L"ProgramModel: Sending signals from QML!!!");
    // submit everything

    qDebug() << "In submitLocalCtrlEx() m_pACtl->dwChangeMask="<<m_pACtl->dwChangeMask;
    lock();
	emit SendCommand((char *)m_pACtl, sizeof(APPLAYLOCALCTRLEX), WTYP_CHANGEMODEEX);
	// clear Change Mask after submit
	m_pACtl->dwChangeMask=0;
    unlock();
}

//================================================================================
//
//================================================================================
void ProgramModel::submitFrequTblAtIdx(int freqIdx) {

    // check index range
	if (freqIdx<0 || freqIdx>MAX_FREQ_IDX) return;
	int iSize = sizeof(FREQUTBL) + sizeof(DWORD);

    // if the frequency value was not changed then quit
    if (!m_pFreqChng[freqIdx]) return;

    lock();
    PFREQUTBL ptbl = (PFREQUTBL)new char[iSize];
	ptbl -> iCount = 1;
	ptbl -> iStartIndx = freqIdx; 
	ptbl -> freq[0].dwFrequ = m_pFtbl->freq[freqIdx].dwFrequ;

    emit SendCommand((char *)ptbl, iSize, WTYP_FREQUCHANGE);
    delete[] (char *)ptbl;

    // clear frequency change flag
    m_pFreqChng[freqIdx]=false;

    unlock();
}

//================================================================================
//
//================================================================================
void ProgramModel::selectTarget() {	
    QMessageBox::information(0,tr("Target selection"),tr("Currently, under construction"));
    return;
    TargetSelection ts(0);
    if(ts.exec()==QDialog::Accepted) {
        emit targetSelected(ts.freqFromHB,ts.freqToHB);
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
