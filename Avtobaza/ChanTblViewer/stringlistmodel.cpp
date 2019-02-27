#include "stringlistmodel.h"

// default contructor: sets bInitialized = false
StringListModel::StringListModel(QObject *parent) 
                       : QAbstractListModel(parent) {
	bInitialized = false;
	iSel=iSec=iFreq=0;
	iParam = PAR_bAttSif;
}

// close (and flush) file on exit
StringListModel::~StringListModel() {
	if (bInitialized) {
		m_fCtrlTblFile.close();
	}
}

// returns private bInitialized property
bool StringListModel::isInitialized() {
	return bInitialized;
}

// returns private bInitialized property
void StringListModel::undoChanges() {
	if (!bInitialized) return;
	beginResetModel();
	ctrlTblCache.clear();
	addToCache();
	emit changesReverted();
	endResetModel();
}

// returns private iParam property
int StringListModel::param() {
	return iParam;
}

// sets private iParam property
void StringListModel::setParam(int iParam) {
	if (!bInitialized) return;
	beginResetModel();
	this->iParam = qBound(0,iParam,PARAMETERS_COUNT-1);
	addToCache();
	endResetModel();
}

// sets private iSel property
void StringListModel::setSel(int iSel) {
	if (!bInitialized) return;
	beginResetModel();
	this->iSel = qBound(0,iSel,ARRAY_SIZE_SEL-1);
	addToCache();
	endResetModel();
}

// sets private iSec property
void StringListModel::setSec(int iSec) {
	if (!bInitialized) return;
	beginResetModel();
	this->iSec = qBound(0,iSec,ARRAY_SIZE_SEC-1);
	addToCache();
	endResetModel();
}

// sets private iFreq property
void StringListModel::setFreq(int iFreq) {
	if (!bInitialized) return;
	beginResetModel();
	this->iFreq = qBound(0,iFreq,ARRAY_SIZE_FREQ-1);
	addToCache();
	endResetModel();
}

// this function is used inside ChanTblViewer::slotSaveCSV()
quint8 StringListModel::bAttWbr(int iSel, int iSec, int iFreq, int iArrIndex) {
	if (!bInitialized) return 0;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);
	iArrIndex = qBound(0, iArrIndex, pusArrayLengths[PAR_bAttWbr]-1);

	return (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWbr[iArrIndex];
}

// this function is used inside ChanTblViewer::slotSaveCSV()
quint8 StringListModel::bAtt(int iSel, int iSec, int iFreq, int iArrIndex) {
	if (!bInitialized) return 0;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);
	iArrIndex = qBound(0, iArrIndex, pusArrayLengths[PAR_bAtt]-1);

	return (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAtt[iArrIndex];
}

// this function is used inside ChanTblViewer::slotSaveCSV()
short StringListModel::sTrash(int iSel, int iSec, int iFreq, int iArrIndex) {
	if (!bInitialized) return 0;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);
	iArrIndex = qBound(0, iArrIndex, pusArrayLengths[PAR_sTrash]-1);

	return (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash[iArrIndex];
}

// this is main initialization function. It initializes pointers, file variable and sets bInitialized=true on success
bool StringListModel::openDatFile(QString fileName) {
	QString str("");

	beginResetModel();

	if (bInitialized) {
		m_fCtrlTblFile.close();
		bInitialized = false;
	}

	int iSize = sizeof(SIFLONGPAPAMS) * ARRAY_SIZE_SEL * ARRAY_SIZE_SEC + sizeof(STROBLONGPARAMSHB) * ARRAY_SIZE_SEL * ARRAY_SIZE_SEC * ARRAY_SIZE_FREQ + sizeof(STROBLONGPARAMSLB) * ARRAY_SIZE_SEL * ARRAY_SIZE_SEC * ARRAY_SIZE_FREQ;

	// set file path, check it exists, check size, open file, map file to memory
	m_fCtrlTblFile.setFileName(fileName);
	if(!m_fCtrlTblFile.exists())
	{	str = QString("File %1 does not exists").arg(m_fCtrlTblFile.fileName());
		emit reportError(str);
		endResetModel();
		return false;
	}
	if(m_fCtrlTblFile.size() != iSize)
	if(!m_fCtrlTblFile.resize(iSize))
	{	str = QString("Failed to resize ctrl tbl file %1").arg(m_fCtrlTblFile.fileName());
		emit reportError(str);
		endResetModel();
		return false;
	}
	if(!m_fCtrlTblFile.open(QIODevice::ReadWrite | QIODevice::Append)) 
	{	str = QString("Failed to open ctrl tbl file %1").arg(m_fCtrlTblFile.fileName());
		emit reportError(str);
		endResetModel();
		return false;
	}

	m_pStrobLongParamsSif = (SIFLONGPAPAMS(*)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC])m_fCtrlTblFile.map(0, iSize);
	if(m_pStrobLongParamsSif == NULL) {
		str = QString("Failed to map ctrl tbl file %1").arg(m_fCtrlTblFile.fileName());
		emit reportError(str);
		endResetModel();
		return false;
	}
	m_fCtrlTblFile.close();

	m_pStrobLongParamsHb = (STROBLONGPARAMSHB(*)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC][ARRAY_SIZE_FREQ])(((char *)m_pStrobLongParamsSif) + ARRAY_SIZE_SEC * ARRAY_SIZE_SEL * sizeof(SIFLONGPAPAMS));
	m_pStrobLongParamsLb = (STROBLONGPARAMSLB(*)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC][ARRAY_SIZE_FREQ])(((char *)m_pStrobLongParamsHb) + ARRAY_SIZE_SEC * ARRAY_SIZE_SEL * ARRAY_SIZE_FREQ * sizeof(STROBLONGPARAMSHB));

	 // pointers are now initialized
	bInitialized = true;

     // this is initial event, need to prepare cache for the model 
	addToCache();

	endResetModel();

	return true;
}

// redefined from QAbstractListModel
QVariant StringListModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
    if (!bInitialized)
         return QVariant();
    if (role != Qt::DisplayRole)
         return QVariant();

    if (orientation == Qt::Horizontal)
         return QString("Column %1").arg(section);
    else
         return QString("Row %1").arg(section);
}

// redefined from QAbstractListModel
Qt::ItemFlags StringListModel::flags(const QModelIndex &index) const {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

// redefined from QAbstractListModel
int StringListModel::rowCount(const QModelIndex &parent) const {
    if (!bInitialized) return 0;

	int iParamIndex = qBound(0,iParam,PARAMETERS_COUNT-1);

	if (iParam != iParamIndex) return 0;
	else return pusArrayLengths[iParamIndex];
}

// redefined from QAbstractListModel
QVariant StringListModel::data(const QModelIndex &index, int role) const {

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		if (!bInitialized)      return QVariant();
		if (!index.isValid())   return QVariant();
		if (iSel != qBound(0, iSel, ARRAY_SIZE_SEL-1)) return QVariant();
		if (iSec != qBound(0, iSec, ARRAY_SIZE_SEC-1)) return QVariant();
		if (iFreq != qBound(0, iFreq, ARRAY_SIZE_FREQ-1)) return QVariant();
		if (iParam != qBound(0, iParam, PARAMETERS_COUNT-1)) return QVariant();

		if (index.row() >= pusArrayLengths[iParam] || index.row() < 0) return QVariant();
		
		ParamKey key;

		key.iSel = iSel;
		key.iSec = iSec;
		key.iFreq = iFreq;
		key.iParam = iParam;
		key.iIndex = index.row();
		// check requested value in cache
		if (ctrlTblCache.contains(key)) return ctrlTblCache[key];
		// cache miss. Load from disk
		else							return QVariant();
	}
    else
        return QVariant();
}

// for keys not contained in cache (ctrlTblCache) -- add them
void StringListModel::addToCache() {
	if (!bInitialized) return;

	ParamKey key;

	key.iSel = iSel;
	key.iSec = iSec;
	key.iFreq = iFreq;
	key.iParam = iParam;

	for (int iIndex=0; iIndex<pusArrayLengths[iParam]; iIndex++) {
		key.iIndex = iIndex;
		if(ctrlTblCache.contains(key)) continue;
		switch (iParam) {
			case PAR_bAttSif:
				ctrlTblCache.insert(key,(*m_pStrobLongParamsSif)[iSel][iSec].bAttSif[key.iIndex]); break;
			case PAR_sTrashSif:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif[key.iIndex]); break;
			case PAR_sTrashSif1:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif1[key.iIndex]); break;
			case PAR_bSifOutMult:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsSif)[iSel][iSec].bSifOutMult[key.iIndex]); break;
			case PAR_bAttWbr:				
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWbr[key.iIndex]); break;
			case PAR_bAtt:				
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAtt[key.iIndex]); break;
			case PAR_sTrash:				
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash[key.iIndex]); break;
			case PAR_sTrash1:				
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash1[key.iIndex]); break;
			case PAR_bWbrOutMult:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbrOutMult[key.iIndex]); break;
			case PAR_bAttL:				
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bAttL[key.iIndex]); break;
			case PAR_sTrashLbr:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr[key.iIndex]); break;
			case PAR_sTrashLbr1:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr1[key.iIndex]); break;
			case PAR_bLbrOutMult:			
		        ctrlTblCache.insert(key,(*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bLbrOutMult[key.iIndex]); break;
			case PAR_bAttWba:				
				ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWba[key.iIndex]); break;
			case PAR_sTrashWba:			
				ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba[key.iIndex]); break;
			case PAR_sTrashWba1:			
				ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba1[key.iIndex]); break;
			case PAR_bWbaOutMult:			
				ctrlTblCache.insert(key,(*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbaOutMult[key.iIndex]); break;
		}
	}
}

// redefined from QAbstractListModel
bool StringListModel::setData(const QModelIndex &index,
                               const QVariant &value, int role) {
    if (!bInitialized) return false;
	if (iSel != qBound(0, iSel, ARRAY_SIZE_SEL-1)) return false;
	if (iSec != qBound(0, iSec, ARRAY_SIZE_SEC-1)) return false;
	if (iFreq != qBound(0, iFreq, ARRAY_SIZE_FREQ-1)) return false;
	if (iParam != qBound(0, iParam, PARAMETERS_COUNT-1)) return false;

    if (index.row() >= pusArrayLengths[iParam] || index.row() < 0)
        return false;

    if (index.isValid() && role == Qt::EditRole) {
		ParamKey key;

		key.iSel = iSel;
		key.iSec = iSec;
		key.iFreq = iFreq;
		key.iParam = iParam;
		key.iIndex = index.row();

		if (!ctrlTblCache.contains(key)) return false;
		
		if (ctrlTblCache[key] == value.toInt()) return false; 

		ctrlTblCache[key] = value.toInt(); 
		
		emit dataChanged(index,index);

		return true;
    }
    return false;
}

void StringListModel::saveToDisk() {
	if (!bInitialized) return;

	foreach (ParamKey key, ctrlTblCache.keys() ) {
		int iParam = key.iParam;
		int iSel   = key.iSel;
		int iSec   = key.iSec;
		int iFreq  = key.iFreq;
		int iIndex = key.iIndex;

		int value  = ctrlTblCache[key];

		if (iParam >= PARAMETERS_COUNT)        continue;
		if (iSel   >= ARRAY_SIZE_SEL)          continue;
		if (iSec   >= ARRAY_SIZE_SEC)          continue;
		if (iFreq  >= ARRAY_SIZE_FREQ)         continue;
		if (iIndex >= pusArrayLengths[iParam]) continue;

		switch (iParam) {
			case PAR_bAttSif:
		        (*m_pStrobLongParamsSif)[iSel][iSec].bAttSif[iIndex] = (quint8)value; break;
			case PAR_sTrashSif:			
		        (*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif[iIndex] = (qint16)value; break;
			case PAR_sTrashSif1:			
		        (*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif1[iIndex] = (qint16)value; break;
			case PAR_bSifOutMult:			
		        (*m_pStrobLongParamsSif)[iSel][iSec].bSifOutMult[iIndex] = (quint8)value; break;
			case PAR_bAttWbr:				
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWbr[iIndex] = (quint8)value; break;
			case PAR_bAtt:				
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAtt[iIndex] = (quint8)value; break;
			case PAR_sTrash:				
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash[iIndex] = (qint16)value; break;
			case PAR_sTrash1:				
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash1[iIndex] = (qint16)value; break;
			case PAR_bWbrOutMult:			
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbrOutMult[iIndex] = (quint8)value; break;
			case PAR_bAttL:				
		        (*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bAttL[iIndex] = (quint8)value; break;
			case PAR_sTrashLbr:			
		        (*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr[iIndex] = (qint16)value; break;
			case PAR_sTrashLbr1:			
		        (*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr1[iIndex] = (qint16)value; break;
			case PAR_bLbrOutMult:			
		        (*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bLbrOutMult[iIndex] = (quint8)value; break;
			case PAR_bAttWba:				
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWba[iIndex] = (quint8)value; break;
			case PAR_sTrashWba:			
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba[iIndex] = (qint16)value; break;
			case PAR_sTrashWba1:			
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba1[iIndex] = (qint16)value; break;
			case PAR_bWbaOutMult:
		        (*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbaOutMult[iIndex] = (quint8)value; break;
			default: 
		        continue;
		}
	}
	emit changesSaved();
}

// this is used inside ChanTblViewer::slotReset()
QString StringListModel::makeSifString(int iSel, int iSec) {
	int i;
	QString str="";
	for (i=0; i<pusArrayLengths[PAR_bAttSif]; i++) { 
		str += QString("%1").arg((*m_pStrobLongParamsSif)[iSel][iSec].bAttSif[i]);
		if (i<pusArrayLengths[PAR_bAttSif]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrashSif]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif[i]);
		if (i<pusArrayLengths[PAR_sTrashSif]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrashSif1]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsSif)[iSel][iSec].sTrashSif1[i]);
		if (i<pusArrayLengths[PAR_sTrashSif1]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_bSifOutMult]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsSif)[iSel][iSec].bSifOutMult[i]);
		if (i<pusArrayLengths[PAR_bSifOutMult]-1) str += QString(" "); else str += QString("\n");
	}
    return str;
}

// this is used inside ChanTblViewer::slotReset()
QString StringListModel::makeWbrString(int iSel, int iSec, int iFreq) {
	int i;

	QString str="";

    if (!bInitialized) return str;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);

	str += QString("%1, ").arg(iFreq);
    for (i=0; i<pusArrayLengths[PAR_bAttWbr]; i++) str += QString("%1, ").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWbr[i]);
	for (i=0; i<pusArrayLengths[PAR_bAtt]; i++) { 
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAtt[i]);
		if (i<pusArrayLengths[PAR_bAtt]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrash]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash[i]);
		if (i<pusArrayLengths[PAR_sTrash]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrash1]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrash1[i]);
		if (i<pusArrayLengths[PAR_sTrash1]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_bWbrOutMult]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbrOutMult[i]);
		if (i<pusArrayLengths[PAR_bWbrOutMult]-1) str += QString(" "); else str += QString("\n");
	}
	return str;
}

// this is used inside ChanTblViewer::slotReset()
QString StringListModel::makeLbrString(int iSel, int iSec, int iFreq) {
	int i;

	QString str="";

    if (!bInitialized) return str;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);

	str += QString("%1, ").arg(iFreq);
	for (i=0; i<pusArrayLengths[PAR_bAttL]; i++) {
		str += QString("%1, ").arg((*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bAttL[i]);
	}
	for (i=0; i<pusArrayLengths[PAR_sTrashLbr]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr[i]);
		if (i<pusArrayLengths[PAR_sTrashLbr]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrashLbr1]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsLb)[iSel][iSec][iFreq].sTrashLbr1[i]);
		if (i<pusArrayLengths[PAR_sTrashLbr1]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_bLbrOutMult]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsLb)[iSel][iSec][iFreq].bLbrOutMult[i]);
		if (i<pusArrayLengths[PAR_bLbrOutMult]-1) str += QString(" "); else str += QString("\n");
	}
	return str;
}

// this is used inside ChanTblViewer::slotReset()
QString StringListModel::makeWbaString(int iSel, int iSec, int iFreq) {
	int i;
	QString str="";

    if (!bInitialized) return str;

	iSel = qBound(0, iSel, ARRAY_SIZE_SEL-1);
	iSec = qBound(0, iSec, ARRAY_SIZE_SEC-1);
	iFreq = qBound(0, iFreq, ARRAY_SIZE_FREQ-1);

	str += QString("%1, ").arg(iFreq);
    for (i=0; i<pusArrayLengths[PAR_bAttWba]; i++) str += QString("%1, ").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bAttWba[i]);
	for (i=0; i<pusArrayLengths[PAR_sTrashWba]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba[i]);
		if (i<pusArrayLengths[PAR_sTrashWba]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_sTrashWba]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].sTrashWba1[i]);
		if (i<pusArrayLengths[PAR_sTrashWba1]-1) str += QString(" "); else str += QString(", ");
	}
	for (i=0; i<pusArrayLengths[PAR_bWbaOutMult]; i++) {
		str += QString("%1").arg((*m_pStrobLongParamsHb)[iSel][iSec][iFreq].bWbaOutMult[i]);
		if (i<pusArrayLengths[PAR_bWbaOutMult]-1) str += QString(" "); else str += QString("\n");
	}
	return str;
}

