#include "stdafx.h"
#include "proppage.h"
#include "qconsolewidget.h"

//*****************************************************************************
//
//*****************************************************************************
ConsolePropPage::ConsolePropPage(QWidget * parent, QConsoleWidget * iparent) : QWidget(parent)
{
	m_pConsole = iparent;
	setupUi(this);

	m_BkgColor = m_pConsole -> m_BkgColor;
	QPalette pl = toolButtonBkgColor -> palette();
	pl.setColor(QPalette::Button, m_BkgColor);
	toolButtonBkgColor -> setPalette(pl);
	spinBoxSpasing -> setValue(m_pConsole -> m_iSpasing);

    checkBoxLog -> setChecked(m_pConsole -> m_bLogEnab);
    spinBoxFileCount -> setValue(m_pConsole -> m_iFileNum);
    lineEditFileName -> setText(m_pConsole -> m_LogFile);
    lineEditFileDir -> setText(m_pConsole -> m_LogDir);

	QAction * pa;
	m_pMenu = new QMenu(this);
	pa = m_pMenu -> addAction(QString("Add style"));
	pa -> setData(STL_ADD);
	pa = m_pMenu -> addAction(QString("Remove style"));
	pa -> setData(STL_REMOVE);

	tableWidgetStyles -> setContextMenuPolicy(Qt::CustomContextMenu);

	int i = 0;
	while(i < m_pConsole -> m_StyleList.count())
	{	TStyle * pst = m_pConsole -> m_StyleList.at(i++);
		m_StyleList.append(new TStyle(pst));
	}
	DrawStyleTbl();
	OnStyleSelectionChanged();
}
//*****************************************************************************
//
//*****************************************************************************
ConsolePropPage::~ConsolePropPage()
{
	DbgPrint(L"Console::PropPage DESTRUCTOR");
	while(m_StyleList.count())
	{	TStyle * p = m_StyleList.first();
		m_StyleList.removeFirst();
		delete p;
	}
	delete m_pMenu;
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnApplayChanges(PDWORD pEFl)
{
	m_pConsole -> lock();
	m_pConsole -> m_BkgColor = m_BkgColor;
	m_pConsole -> m_iSpasing = spinBoxSpasing -> value();
	while(m_pConsole -> m_StyleList.count())
	{	TStyle * p = m_pConsole -> m_StyleList.first();
		m_pConsole -> m_StyleList.removeFirst();
		delete p;
	}
	while(m_StyleList.count())
	{	TStyle * p = m_StyleList.first();
		m_StyleList.removeFirst();
		m_pConsole -> m_StyleList.append(p);
	}
	m_pConsole -> unlock();

	bool bLogEnab = m_pConsole -> m_bLogEnab;
	QString qsDir = m_pConsole -> m_LogDir;
	QString qsFile = m_pConsole -> m_LogFile;
	int iCount = m_pConsole -> m_iFileNum;

	m_pConsole -> m_bLogEnab = checkBoxLog -> isChecked();
	m_pConsole -> m_LogDir = lineEditFileDir -> text();
	m_pConsole -> m_LogFile = lineEditFileName -> text();
	m_pConsole -> m_iFileNum = spinBoxFileCount -> value();
	
	if(bLogEnab != m_pConsole -> m_bLogEnab || qsDir != m_pConsole -> m_LogDir ||
		qsFile != m_pConsole -> m_LogFile || iCount != m_pConsole -> m_iFileNum)
	{	if(bLogEnab) m_pConsole -> StopLoging();
		if(m_pConsole -> m_bLogEnab) m_pConsole -> StartLoging();
	}
	m_pConsole -> update();
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnButtonBkgColorClicked(void)
{	QColor clr;
	
	clr = QColorDialog::getColor(m_BkgColor, this);
	if(clr.isValid())
	{	m_BkgColor = clr;
		QPalette pl = toolButtonBkgColor -> palette();
		pl.setColor(QPalette::Button, clr);
		toolButtonBkgColor -> setPalette(pl);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnButtonColorClicked(void)
{	QColor clr, * pclr;
	QToolButton * ptb = (QToolButton *)sender(); 

	QList<QTableWidgetItem *> il = tableWidgetStyles -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TStyle * ps = m_StyleList.at(iCurSel);
	
	if(ptb == toolButtonBkgColorS) pclr = &ps -> m_BkgColor;
	else if(ptb == toolButtonTextColor) pclr = &ps -> m_TextColor; 
	else pclr = &clr;

	QColorDialog cld(*pclr, this);
	cld.setOption(QColorDialog::ShowAlphaChannel, true);	
	if(cld.exec())
	{	clr = cld.selectedColor();
		*pclr = clr; 
		clr.setAlpha(255);
		QPalette pl = ptb -> palette();
		pl.setColor(QPalette::Button, clr);
		ptb -> setPalette(pl);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnButtonFontClicked(void)
{
	QList<QTableWidgetItem *> il = tableWidgetStyles -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TStyle * ps = m_StyleList.at(iCurSel);
	QFontDialog fntd(ps -> m_Font, this); 
	if(fntd.exec())
	{	ps -> m_Font = fntd.selectedFont();
		pushButtonFont -> setFont(ps -> m_Font);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnCustomContextMenuRequest(QPoint pos)
{	int i;
	QAction * pa;
	QMenu * pm = m_pMenu;
	QTableWidgetItem * pCurItem = NULL;
	int iRow = -1;

	QList<QTableWidgetItem *> list = tableWidgetStyles -> selectedItems();
	if(list.count() != 0) 
	{	pCurItem = list.first();
		iRow = pCurItem -> row();
	}
//	TStyle * ps = NULL;
//	if(iRow != -1) ps = m_StyleList.at(iRow);
	QList<QAction *> alist = pm -> actions();
	i = 0;
	while(i < alist.count())
	{	pa = alist.at(i++);
		switch(pa -> data().toInt())
		{	case STL_ADD:
				pa -> setEnabled(true);
				break;

			case STL_REMOVE:
				pa -> setEnabled(iRow != -1);
				break;

		}
	}
	pa = pm -> exec(QCursor::pos());
	if(pa == NULL) return;
	switch(pa -> data().toUInt())
	{	case STL_ADD:
			AddStyle(iRow);
			break;

		case STL_REMOVE:
			RemoveStyle(iRow);
			break;

		
		default:
			DbgPrint(L"ConsolePropPage::OnCustomContextMenuRequest Unknown ID=%ld", pa -> data().toUInt());
			break;

	}
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::AddStyle(int iRow)
{
	if(iRow == -1) m_StyleList.append(new TStyle());
	else  m_StyleList.insert(iRow, new TStyle());
	DrawStyleTbl();
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::RemoveStyle(int iRow)
{
	if(iRow == -1) return;
	TStyle * ps = m_StyleList.at(iRow);
	m_StyleList.removeAt(iRow);
	delete ps;
	DrawStyleTbl();
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::DrawStyleTbl(void)
{	QString qs;
	QTableWidgetItem * pItem;

	tableWidgetStyles -> clearContents();
	tableWidgetStyles -> setRowCount(m_StyleList.count());
	
	int i = 0;
	while(i < m_StyleList.count())
	{	TStyle * ps = m_StyleList.at(i);
		
		pItem = new QTableWidgetItem();					// Id
		pItem -> setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		qs.setNum(ps -> m_iId);
		pItem -> setText(qs);
		tableWidgetStyles -> setItem(i, 0, pItem);
		
		pItem = new QTableWidgetItem();					// Enable
		pItem -> setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
		pItem -> setText("");
		if(ps -> m_bEnabled) pItem -> setCheckState(Qt::Checked);
		else pItem -> setCheckState(Qt::Unchecked);
		tableWidgetStyles -> setItem(i, 1, pItem);
		i++;
	}
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnStyleIdChage(QTableWidgetItem * pItem)
{
	TStyle * ps = m_StyleList.at(pItem -> row());
	if(pItem -> column() == 0) ps -> m_iId = pItem -> text().toInt();
	if(pItem -> column() == 1) ps -> m_bEnabled = pItem -> checkState() == Qt::Checked;
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnStyleSelectionChanged(void)
{
	QPalette pl;
	QList<QTableWidgetItem *> il = tableWidgetStyles -> selectedItems();
	if(il.count() == 0)
	{	groupBoxMessage -> setEnabled(false);
		pl = toolButtonBkgColorS -> palette();
		pl.setColor(QPalette::Button, Qt::gray);
		toolButtonBkgColorS -> setPalette(pl);
		pl = toolButtonTextColor -> palette();
		pl.setColor(QPalette::Button, Qt::gray);
		toolButtonTextColor -> setPalette(pl);
		return;
	}
	int iCurSel = il.first() -> row();
	TStyle * ps = m_StyleList.at(iCurSel);
	groupBoxMessage -> setEnabled(true);
	pl = toolButtonBkgColorS -> palette();
	pl.setColor(QPalette::Button, ps -> m_BkgColor);
	toolButtonBkgColorS -> setPalette(pl);
	pl = toolButtonTextColor -> palette();
	pl.setColor(QPalette::Button, ps -> m_TextColor);
	toolButtonTextColor -> setPalette(pl);
	pushButtonFont -> setFont(ps -> m_Font);
}
//*****************************************************************************
//
//*****************************************************************************
void ConsolePropPage::OnDirectoryButtonClicked(void)
{
	QFileDialog * pfd;

	pfd = new QFileDialog(this, QString((QChar *)L"Log to directory"));
	pfd -> setFileMode(QFileDialog::DirectoryOnly);
	pfd -> setViewMode(QFileDialog::Detail);
	pfd -> selectFile(lineEditFileDir -> text());

	if(!pfd -> exec())
	{	delete pfd;
		return;
	}
	QDir curdir=QDir::current();
	QString qsd = curdir.relativeFilePath(pfd -> selectedFiles().at(0));
	if(qsd.isNull()) qsd = ".";
	lineEditFileDir -> setText(qsd);
	delete pfd;
}
