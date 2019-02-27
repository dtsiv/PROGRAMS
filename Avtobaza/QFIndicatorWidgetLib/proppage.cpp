#include "stdafx.h"
#include "proppage.h"
#include "qfindicatorwidget.h"
#include "findicatorline.h"

//*****************************************************************************
//
//*****************************************************************************
FIndicatorPropPage::FIndicatorPropPage(QWidget * parent, QFIndicatorWidget * iparent) : QWidget(parent)
{
	m_pFIndicator = iparent;
	setupUi(this);
	checkBoxEnableRequests -> setChecked(m_pFIndicator -> m_bRequestsEnab);
	
	QAction * pa;
	m_pMenu = new QMenu(this);
	pa = m_pMenu -> addAction(QString("Add F-Line"));
	pa -> setData(STL_ADD);
	pa = m_pMenu -> addAction(QString("Remove F-Line"));
	pa -> setData(STL_REMOVE);

	tableWidgetFLines -> setContextMenuPolicy(Qt::CustomContextMenu);

	m_pFIndicator -> lock();
	int i = 0;
	while(i < m_pFIndicator -> m_FLineList.count())
	{	TFIndicatorLine * pfl = m_pFIndicator -> m_FLineList.at(i++);
		m_StyleList.append(new TFIndicatorLineStyle(pfl));
	}
	m_pFIndicator -> unlock();
	DrawFLinesTbl();
	OnFLineSelectionChanged();
}
//*****************************************************************************
//
//*****************************************************************************
FIndicatorPropPage::~FIndicatorPropPage()
{

	while(m_StyleList.count())
	{	TFIndicatorLineStyle * p = m_StyleList.first();
		m_StyleList.removeFirst();
		delete p;
	}
	delete m_pMenu;				
	
	DbgPrint(L"FIndicatorPropPage::PropPage DESTRUCTOR");
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnApplayChanges(PDWORD pEFl)
{
	m_pFIndicator -> m_bRequestsEnab = checkBoxEnableRequests -> isChecked();
	QTableWidgetItem * pItem;
	int i = 0;
	while(i < m_StyleList.count())
	{	TFIndicatorLineStyle * pl = m_StyleList.at(i);
		pItem = tableWidgetFLines -> item(i, 0);					// FFrom
		pl -> m_dFFrom = pItem -> text().toDouble();
		pItem = tableWidgetFLines -> item(i, 1);					// FTo
		pl -> m_dFTo = pItem -> text().toDouble();
		pItem = tableWidgetFLines -> item(i, 2);					// YMax
		pl -> m_dYMax = pItem -> text().toDouble();
		i++;
	}
	m_pFIndicator -> ApplyChanges(&m_StyleList);
	DbgPrint(L"FIndicatorPropPage::OnApplayChanges errFl=%lx", *pEFl);
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnButtonBkgColorClicked(void)
{	QColor clr;
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	
	clr = QColorDialog::getColor(ps -> m_BkgColor, this);
	if(clr.isValid())
	{	ps -> m_BkgColor = clr;
		QPalette pl = toolButtonBkgColor -> palette();
		pl.setColor(QPalette::Button, clr);
		toolButtonBkgColor -> setPalette(pl);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnButtonItemColorClicked(void)
{	QColor clr, * pclr;
	QToolButton * ptb = (QToolButton *)sender(); 

	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	if(ptb == toolButtonFillColor) pclr = &ps -> m_clrFill;	
	else if(ptb == toolButtonOutlineColor) pclr = &ps -> m_clrOutline;	
	else if(ptb == toolButtonTextColor) pclr = &ps -> m_clrText;
	else if(ptb == toolButtonTColor)	pclr = &ps -> m_clrTrace;
	else if(ptb == toolButtonPoitColor) pclr = &ps -> m_clrPoit;
	else if(ptb == toolButtonVertLineColor) pclr = &ps -> m_clrLineVertic;
	else if(ptb == toolButtonVertTextColor) pclr = &ps -> m_clrTextVertic;
	else if(ptb == toolButtonHorizLineColor) pclr = &ps -> m_clrLineHoriz;
	else if(ptb == toolButtonHorizTextColor) pclr = &ps -> m_clrTextHoriz;
	else if(ptb == toolButtonP0Color) pclr = &ps -> m_clrP0;
	else if(ptb == toolButtonP1Color) pclr = &ps -> m_clrP1;
	else if(ptb == toolButtonP2Color) pclr = &ps -> m_clrP2;
	else if(ptb == toolButtonP3Color) pclr = &ps -> m_clrP3;
	
	else return;

	clr = QColorDialog::getColor(*pclr, this, "Select color", QColorDialog::ShowAlphaChannel);
	if(clr.isValid())
	{	*pclr = clr;
		QPalette pl = ptb -> palette();
		clr.setAlpha(255);
		pl.setColor(QPalette::Button, clr);
		ptb -> setPalette(pl);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnButtonFontClicked(void)
{
	QPushButton * ppb = (QPushButton *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
    QFont * pFont;
	if(ppb == pushButtonFontW) pFont = &ps -> m_WFont;
	else pFont = &ps -> m_FFont;
	
	QFontDialog fntd(*pFont, this); 
	if(fntd.exec())
	{	*pFont = fntd.selectedFont();
		ppb -> setFont(*pFont);
	}
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnIdEditingFinished(void)
{	
	QLineEdit * ple = (QLineEdit *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	int * pi;
	if(ple == lineEditId0) pi = &ps -> m_PId0;
	else if(ple == lineEditId1) pi = &ps -> m_PId1;
	else if(ple == lineEditId2) pi = &ps -> m_PId2;
	else if(ple == lineEditId3) pi = &ps -> m_PId3;
	else if(ple == lineEditComPoints) pi = &ps -> m_iComPoints;
	else return;
	*pi = ple -> text().toInt();

}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnCustomContextMenuRequest(QPoint pos)
{	int i;
	QAction * pa;
	QMenu * pm = m_pMenu;
	QTableWidgetItem * pCurItem = NULL;
	int iRow = -1;

	QList<QTableWidgetItem *> list = tableWidgetFLines -> selectedItems();
	if(list.count() != 0) 
	{	pCurItem = list.first();
		iRow = pCurItem -> row();
	}
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
			DbgPrint(L"FIndicatorPropPage::OnCustomContextMenuRequest Unknown ID=%ld", pa -> data().toUInt());
			break;

	}
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::AddStyle(int iRow)
{
	if(iRow == -1) m_StyleList.append(new TFIndicatorLineStyle());
	else  m_StyleList.insert(iRow, new TFIndicatorLineStyle());
	DrawFLinesTbl();
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::RemoveStyle(int iRow)
{
	if(iRow == -1) return;
	TFIndicatorLineStyle * ps = m_StyleList.at(iRow);
	m_StyleList.removeAt(iRow);
	delete ps;
	DrawFLinesTbl();
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::DrawFLinesTbl(void)
{	QString qs;
	QTableWidgetItem * pItem;

	tableWidgetFLines -> clearContents();
	tableWidgetFLines -> setRowCount(m_StyleList.count());
	
	int i = 0;
	while(i < m_StyleList.count())
	{	TFIndicatorLineStyle * ps = m_StyleList.at(i);
		
		pItem = new QTableWidgetItem();					// FFrom
		pItem -> setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		qs.setNum(ps -> m_dFFrom, 'f', 1);
		pItem -> setText(qs);
		tableWidgetFLines -> setItem(i, 0, pItem);
		
		pItem = new QTableWidgetItem();					// FTo
		pItem -> setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		qs.setNum(ps -> m_dFTo, 'f', 1);
		pItem -> setText(qs);
		tableWidgetFLines -> setItem(i, 1, pItem);

		pItem = new QTableWidgetItem();					// YMax
		pItem -> setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		qs.setNum(ps -> m_dYMax, 'f', 1);
		pItem -> setText(qs);
		tableWidgetFLines -> setItem(i, 2, pItem);
		i++;
	}

}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnFLineSelectionChanged(void)
{
	QToolButton * ptb[] = { toolButtonBkgColor, toolButtonFillColor, toolButtonOutlineColor,
		toolButtonTextColor, toolButtonTColor, toolButtonPoitColor,
		toolButtonVertLineColor, toolButtonVertTextColor, toolButtonHorizLineColor,
		toolButtonHorizTextColor, toolButtonP0Color, toolButtonP1Color, toolButtonP2Color,
		toolButtonP3Color };

	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0)
	{	groupBoxFormular -> setEnabled(false);
		spinBoxPointTTL -> setEnabled(false);
		pushButtonFontW -> setEnabled(false);
		labelTTL -> setEnabled(false);
		labelBkgColor -> setEnabled(false);
		checkBoxShowVert -> setEnabled(false);
		checkBoxShowHoriz -> setEnabled(false);
		checkBoxShowVert -> setChecked(false);
		checkBoxShowHoriz -> setChecked(false);
		checkBoxShowTraces -> setChecked(false); 
		checkBoxShowPoit -> setChecked(false);
		checkBoxShowTraces -> setEnabled(false);
		checkBoxShowPoit -> setEnabled(false);
		checkBoxShowPeleng0 -> setChecked(false);
		checkBoxShowPeleng1 -> setChecked(false);
		checkBoxShowPeleng2 -> setChecked(false);
		checkBoxShowPeleng3 -> setChecked(false);
		checkBoxTaRequest -> setEnabled(false);
		checkBoxTaRequest -> setChecked(false);
		checkBoxSowFFT -> setEnabled(false);
		checkBoxSowFFT -> setChecked(false);
		labelDencityX -> setEnabled(false);
		labelDencityY -> setEnabled(false);
		spinBoxDencityX -> setEnabled(false);
		spinBoxDencityY -> setEnabled(false);
		comboBoxFormular -> setCurrentIndex(0);
		groupBoxShowPeleng -> setEnabled(false);
		radioButtonPelengs -> setEnabled(false);
		radioButtonRaw -> setEnabled(false);
		lineEditComPoints -> setEnabled(false);
		labelAzFrom -> setEnabled(false);
		labelAzTo -> setEnabled(false);
		labelComPoints -> setEnabled(false);
		doubleSpinBoxAzFrom -> setEnabled(false);
		doubleSpinBoxAzTo -> setEnabled(false);

		lineEditId0 -> setText("");
		lineEditId1 -> setText("");
		lineEditId2 -> setText("");
		lineEditId3 -> setText("");
		lineEditComPoints -> setText("");

		int i = 0;
		while(i < 14)
		{	QPalette pl = ptb[i] -> palette();
			pl.setColor(QPalette::Button, Qt::gray);
			ptb[i++] -> setPalette(pl);
		}
		return;
	}
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	spinBoxPointTTL -> setEnabled(true);
	labelTTL -> setEnabled(true);
	spinBoxPointTTL -> setValue(ps -> m_iTTL);
	groupBoxFormular -> setEnabled(true);
	groupBoxShowPeleng -> setEnabled(true);

	labelBkgColor -> setEnabled(true);
	checkBoxShowVert -> setEnabled(true);
	checkBoxShowHoriz -> setEnabled(true);
	checkBoxShowTraces -> setEnabled(true);
	checkBoxShowPoit -> setEnabled(true);
	checkBoxTaRequest -> setEnabled(true);
	checkBoxSowFFT -> setEnabled(true);
	radioButtonPelengs -> setEnabled(true);
	radioButtonRaw -> setEnabled(true);

	checkBoxShowVert -> setChecked(ps -> m_EnabMask & ENAB_VERTIC);
	checkBoxShowHoriz -> setChecked(ps -> m_EnabMask & ENAB_HORIZ);
	checkBoxShowTraces -> setChecked(ps -> m_EnabMask & ENAB_SHOW_T);
	checkBoxShowPoit -> setChecked(ps -> m_EnabMask & ENAB_SHOW_POIT);
	checkBoxShowPeleng0 -> setChecked(ps -> m_EnabMask & ENAB_SHOW_P0);
	checkBoxShowPeleng1 -> setChecked(ps -> m_EnabMask & ENAB_SHOW_P1);
	checkBoxShowPeleng2 -> setChecked(ps -> m_EnabMask & ENAB_SHOW_P2);
	checkBoxShowPeleng3 -> setChecked(ps -> m_EnabMask & ENAB_SHOW_P3);
	checkBoxTaRequest -> setChecked(ps -> m_EnabMask & ENAB_TA_REQUEST);
	checkBoxSowFFT -> setChecked(ps -> m_EnabMask & ENAB_SHOW_FFT);

	radioButtonPelengs -> setChecked(!(ps -> m_EnabMask & ENAB_SHOW_RAW));
	radioButtonRaw -> setChecked(ps -> m_EnabMask & ENAB_SHOW_RAW);

	labelDencityX -> setEnabled(true);
	labelDencityY -> setEnabled(true);
	spinBoxDencityX -> setEnabled(true);
	spinBoxDencityY -> setEnabled(true);
	spinBoxDencityX -> setValue(ps -> m_WebDencityX);
	spinBoxDencityY -> setValue(ps -> m_WebDencityY);
	QString qs;
	
	lineEditId0 -> setText(qs.setNum(ps -> m_PId0));
	lineEditId1 -> setText(qs.setNum(ps -> m_PId1));
	lineEditId2 -> setText(qs.setNum(ps -> m_PId2));
	lineEditId3 -> setText(qs.setNum(ps -> m_PId3));

	labelComPoints -> setEnabled(true);
	lineEditComPoints -> setEnabled(true);
	lineEditComPoints -> setText(qs.setNum(ps -> m_iComPoints));

	labelAzFrom -> setEnabled(true);
	labelAzTo -> setEnabled(true);
	doubleSpinBoxAzFrom -> setEnabled(true);
	doubleSpinBoxAzTo -> setEnabled(true);
	doubleSpinBoxAzFrom -> setValue(ps -> m_dAzFrom);
	doubleSpinBoxAzTo -> setValue(ps -> m_dAzTo);

	QColor * pclr[] = {	&ps -> m_BkgColor, &ps -> m_clrFill, &ps -> m_clrOutline,	
		&ps -> m_clrText, &ps -> m_clrTrace, &ps -> m_clrPoit, &ps -> m_clrLineVertic,
		&ps -> m_clrTextVertic, &ps -> m_clrLineHoriz, &ps -> m_clrTextHoriz,
		&ps -> m_clrP0, &ps -> m_clrP1, &ps -> m_clrP2, &ps -> m_clrP3 };
	
	int i = 0;
	while(i < 14)
	{	QPalette pl = ptb[i] -> palette();
		pl.setColor(QPalette::Button, *pclr[i]);
		ptb[i++] -> setPalette(pl);
	}
	pushButtonFontW -> setEnabled(true);
	pushButtonFontW -> setFont(ps -> m_WFont);
	pushButtonFontF -> setFont(ps -> m_FFont);
	comboBoxFormular -> setCurrentIndex(ps -> m_iFormType);
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnFormularTypeChanged(int iType)
{
	QCheckBox * pcb = (QCheckBox *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	ps -> m_iFormType = iType;
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnCheckBoxShowChanged()
{
	QCheckBox * pcb = (QCheckBox *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	int iMask;
	if(pcb == checkBoxShowVert) iMask = ENAB_VERTIC;	
	if(pcb == checkBoxShowHoriz) iMask = ENAB_HORIZ;	
	if(pcb == checkBoxShowTraces) iMask = ENAB_SHOW_T;	
	if(pcb == checkBoxShowPoit) iMask = ENAB_SHOW_POIT;	
	if(pcb == checkBoxShowPeleng0) iMask = ENAB_SHOW_P0;
	if(pcb == checkBoxShowPeleng1) iMask = ENAB_SHOW_P1;
	if(pcb == checkBoxShowPeleng2) iMask = ENAB_SHOW_P2;
	if(pcb == checkBoxShowPeleng3) iMask = ENAB_SHOW_P3;
	if(pcb == checkBoxTaRequest) iMask = ENAB_TA_REQUEST;
	if(pcb == checkBoxSowFFT) iMask = ENAB_SHOW_FFT;

	if(pcb -> isChecked()) ps -> m_EnabMask |= iMask;
	else ps -> m_EnabMask &= ~iMask;
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnRadioButtunClicked(void)
{
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	QRadioButton * prb = (QRadioButton *)sender();
	if(prb == radioButtonRaw)
	{	if(prb -> isChecked()) ps -> m_EnabMask |= ENAB_SHOW_RAW;
		else ps -> m_EnabMask &= ~ENAB_SHOW_RAW;
	} else
	{	if(!prb -> isChecked()) ps -> m_EnabMask |= ENAB_SHOW_RAW;
		else ps -> m_EnabMask &= ~ENAB_SHOW_RAW;
	}
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnDensValueChanged(int iVal)
{
	QSpinBox * psb = (QSpinBox *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	if(psb == spinBoxDencityX) ps -> m_WebDencityX = iVal;
	if(psb == spinBoxDencityY) ps -> m_WebDencityY = iVal;
	if(psb == spinBoxPointTTL) ps -> m_iTTL = iVal;
}
//*****************************************************************************
//
//*****************************************************************************
void FIndicatorPropPage::OnAzValueChanged(double dVal)
{
	QDoubleSpinBox * psb = (QDoubleSpinBox *)sender(); 
	QList<QTableWidgetItem *> il = tableWidgetFLines -> selectedItems();
	if(il.count() == 0) return;
	int iCurSel = il.first() -> row();
	TFIndicatorLineStyle * ps = m_StyleList.at(iCurSel);
	if(psb == doubleSpinBoxAzFrom) ps -> m_dAzFrom = dVal;
	if(psb == doubleSpinBoxAzTo) ps -> m_dAzTo = dVal;
}
