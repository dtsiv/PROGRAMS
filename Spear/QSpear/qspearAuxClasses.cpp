//================= Aux classes =================
#include "qspear.h"
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRmoValidator::fixup (QString &input) const {
	int iPos;
	if(QRegExpValidator::validate(input,iPos) != QValidator::Acceptable)
		input=m_qsDefault;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
QRmoLineEdit::QRmoLineEdit(const QString & contents, 
			ContentTypes type /*=ctArbitrary */,
			QWidget * parent /*= 0*/) :
		QLineEdit(contents, parent) {
	if (type & ctFixedPoint) {  // apply fixed-point number validator
		QRegExp rx("^\\s*[-+]?\\d+\\.?\\d*\\s*$");
		setValidator(new QRmoValidator(rx, "0.0", this));
	}
	QString qsPlaceholder=contents;
	switch(type) {
		case (ctFixedPoint):  // unspecific double
		case (ctInteger):     // unspecific integer
			QObject::connect(this,SIGNAL(editingFinished()),this,SLOT(onEditingFinished()));
			break;
		case (ctFrequency):
			QObject::connect(this,SIGNAL(editingFinished()),this,SLOT(onFreqEditingFinished()));
			qsPlaceholder=QSpearModel::placeholderFreq;
			break;
		default:
			// no input checking by default
			break;
	}
	setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	setFixedWidth(fontMetrics().width(qsPlaceholder)+10);
	setText(text().trimmed());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRmoLineEdit::onEditingFinished() {
	setText(text().trimmed());
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRmoLineEdit::onFreqEditingFinished() {
	setText(QSpearModel::formatFreqString(text()));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void QRmoLineEdit::focusInEvent ( QFocusEvent * event ) {
	// First let the base class process the event
	QLineEdit::focusInEvent(event);
	// Then select the text by a single shot timer, so that everything will
	// be processed before (calling selectAll() directly won't work)
	QTimer::singleShot(0, this, SLOT(selectAll()));
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UserControlInputFilter::UserControlInputFilter(QSpear *pOwner, QObject *pobj /*=0*/) 
    : QObject(pobj), m_pOwner(pOwner) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*virtual*/ bool UserControlInputFilter::eventFilter(QObject *pobj, QEvent *pe) {
	QEvent::Type eventType = pe->type();
	// filer only mainWindow events
    if (pobj!=m_pOwner && pobj!=m_pOwner->m_pslRotationSpeed) return false;
    // user input events
	if (eventType == QEvent::KeyPress) {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pe;
		if ((pKeyEvent->key()==Qt::Key_Q || pKeyEvent->key()==Qt::Key_X) 
			   && (pKeyEvent->modifiers() & Qt::ControlModifier || pKeyEvent->modifiers() & Qt::AltModifier)) {
		    qApp->quit();
            return true;
		}
        if (m_pOwner->m_pgbJustirovka->isEnabled()) {
            if (pKeyEvent->key()==Qt::Key_Up) {
                m_pOwner->m_ppbRotUp->click();
                return true;
            }
            if (pKeyEvent->key()==Qt::Key_Down) {
                m_pOwner->m_ppbRotDown->click();
                return true;
            }
            if (pKeyEvent->key()==Qt::Key_Right) {
                m_pOwner->m_ppbRotRight->click();
                return true;
            }
            if (pKeyEvent->key()==Qt::Key_Left) {
                m_pOwner->m_ppbRotLeft->click();
                return true;
            }
        }
	}
    // allow all other events
	return false;
}
