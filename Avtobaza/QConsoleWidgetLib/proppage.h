#ifndef QRMOPROPPAGE_H
#define QRMOPROPPAGE_H

#include <QtCore>
#include <QMenu>
#include <QPoint>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>

#include "codograms_and_qavtctrl.h"
#include "ui_proppage.h"

#define STL_ADD		200
#define STL_REMOVE	201

class QConsoleWidget; 
//*****************************************************************************
//
//*****************************************************************************
class TStyle
{
public:
	TStyle()
	{	m_iId = 0;
		m_BkgColor = QColor(255, 255, 255, 0);
		m_TextColor = QColor(0, 0, 0);
		m_bEnabled = true;
	}
	TStyle(TStyle * other) { *this = *other; }

public:
	int m_iId;
	bool m_bEnabled;
	QColor m_TextColor;
	QColor m_BkgColor;
	QFont m_Font;
};
//*****************************************************************************
//
//*****************************************************************************
class ConsolePropPage : public QWidget, public Ui::QConsolePropPage
{
	Q_OBJECT

public:
	ConsolePropPage(QWidget * parent, QConsoleWidget * iparent);
	~ConsolePropPage();

public slots:
	void OnCustomContextMenuRequest(QPoint pos);
	void OnApplayChanges(PDWORD pEFl);
	void OnButtonColorClicked(void);
	void OnButtonFontClicked(void);
	void OnStyleIdChage(QTableWidgetItem * pItem);
	void OnStyleSelectionChanged(void);
	void OnButtonBkgColorClicked(void);
	void OnDirectoryButtonClicked(void);

protected:

private:
	QConsoleWidget * m_pConsole;
	QColor m_BkgColor;
	QMenu * m_pMenu;
	QList<TStyle *> m_StyleList;
	void AddStyle(int iRow);
	void RemoveStyle(int iRow); 
	void DrawStyleTbl(void);

public:
};
#endif // QRMOPROPPAGE_H
