#ifndef STRINGLISTMODEL_H
#define STRINGLISTMODEL_H

#include <QtGui>

#ifdef __linux
    #include "qavtctrl_linux.h"
    #include "codograms_linux.h"
#else
    #include "qavtctrl.h"
    #include "codograms.h"
#endif

#define     ARRAY_SIZE_SEL          32
#define     ARRAY_SIZE_SEC          8
#define     ARRAY_SIZE_FREQ         2048

#define		PAR_bAttSif				0
#define		PAR_sTrashSif			1
#define		PAR_sTrashSif1			2
#define		PAR_bSifOutMult			3
#define		PAR_bAttWbr				4
#define		PAR_bAtt				5
#define		PAR_sTrash				6
#define		PAR_sTrash1				7
#define		PAR_bWbrOutMult			8
#define		PAR_bAttL				9
#define		PAR_sTrashLbr			10
#define		PAR_sTrashLbr1			11
#define		PAR_bLbrOutMult			12
#define		PAR_bAttWba				13
#define		PAR_sTrashWba			14
#define		PAR_sTrashWba1			15
#define		PAR_bWbaOutMult			16
// keep PARAMETERS_COUNT as standalone named constant
#define		PARAMETERS_COUNT		17

// to control array index range in StringListModel::data, StringListModel::setData
const unsigned short pusArrayLengths[PARAMETERS_COUNT]
	= { 4, 4, 4, 4, 2, 14, 14, 14, 14, 2, 2, 2, 2, 2, 16, 16, 2 };

// key for internal cache based on QMap
typedef struct PARAMKEY_ {
    quint8		iSel;
    quint8		iSec;
    quint16		iFreq;
    quint8		iParam;
	quint8      iIndex;
} ParamKey;

// ordering operator - used to build keys index
inline bool operator<(const ParamKey &k1, const ParamKey &k2) {
    QString sk1 = QString("%1%2%3%4%5")
            .arg(k1.iSel,2)
            .arg(k1.iSec,1)
            .arg(k1.iFreq,4)
            .arg(k1.iParam,2)
            .arg(k1.iIndex,2);
    QString sk2 = QString("%1%2%3%4%5")
            .arg(k2.iSel,2)
            .arg(k2.iSec,1)
            .arg(k2.iFreq,4)
            .arg(k2.iParam,2)
            .arg(k2.iIndex,2);
    return sk1 < sk2;
}

class StringListModel : public QAbstractListModel
{
	Q_OBJECT
private:
	SIFLONGPAPAMS (* m_pStrobLongParamsSif)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC];
	STROBLONGPARAMSHB (* m_pStrobLongParamsHb)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC][ARRAY_SIZE_FREQ];
	STROBLONGPARAMSLB (* m_pStrobLongParamsLb)[ARRAY_SIZE_SEL][ARRAY_SIZE_SEC][ARRAY_SIZE_FREQ];

	int iSel, iSec, iFreq;
	int iParam;

	bool bInitialized;

	QFile m_fCtrlTblFile;

	QMap<ParamKey,qint32> ctrlTblCache;

public:
	StringListModel(QObject *parent=0);

	~StringListModel();

	bool openDatFile(QString fileName);

	Qt::ItemFlags flags(const QModelIndex &) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	int rowCount(const QModelIndex &parent) const;
	bool isInitialized();
 	QString makeSifString(int iSel, int iSec);
 	QString makeWbrString(int iSel, int iSec, int iFreq);
 	QString makeLbrString(int iSel, int iSec, int iFreq);
 	QString makeWbaString(int iSel, int iSec, int iFreq);
	quint8 bAttWbr(int iSel, int iSec, int iFreq, int iArrIndex);
	quint8 bAtt(int iSel, int iSec, int iFreq, int iArrIndex);
	short sTrash(int iSel, int iSec, int iFreq, int iArrIndex);
	int param();
	void setParam(int iParam);
	void addToCache(void);

signals:
	void reportError(QString);
	void changesSaved();
	void changesReverted();

public slots:
	void setSel(int iSel);
	void setSec(int iSec);
	void setFreq(int iFreq);
	void saveToDisk();
	void undoChanges();

};

#endif // STRINGLISTMODEL_H
