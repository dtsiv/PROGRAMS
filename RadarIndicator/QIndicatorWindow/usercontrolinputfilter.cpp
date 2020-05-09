#include "usercontrolinputfilter.h"
#include "qindicatorwindow.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UserControlInputFilter::UserControlInputFilter(QIndicatorWindow *pOwner, QObject *pobj /*=0*/)
    : QObject(pobj), m_pOwner(pOwner) {
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*virtual*/ bool UserControlInputFilter::eventFilter([[maybe_unused]]QObject *pobj, QEvent *pe) {
    QEvent::Type eventType = pe->type();
    // block all user input events during parsing
    if (m_pOwner->m_bParsingInProgress==true || m_pOwner->m_bGenerateNoiseMapInProgress) {
        if (    eventType == QEvent::KeyPress
             || eventType == QEvent::KeyRelease
             || eventType == QEvent::MouseButtonPress
             || eventType == QEvent::MouseButtonRelease
             || eventType == QEvent::MouseButtonDblClick) {
            // block all possible events. Which else?...
            return true;
        }
    }
    // common user input events
    if (eventType == QEvent::KeyPress) {
        QKeyEvent *pKeyEvent = (QKeyEvent*)pe;
        // qDebug() << "key= " << pKeyEvent->key();
        // qDebug() << "pKeyEvent->modifiers()= " << QString::number((int)pKeyEvent->modifiers(),16);
        if (pKeyEvent->key()==Qt::Key_Return) {
            // block all other events
            return true;
        }
        else if ((pKeyEvent->key()==Qt::Key_P)
               && (pKeyEvent->modifiers() & Qt::ControlModifier)) {
            m_pOwner->onSetup();
            return true;
        }
        else if (pKeyEvent->key()==Qt::Key_Escape) {
            // m_pOwner->closeView();
            return true;
        }
        else if ((pKeyEvent->key()==Qt::Key_Q || pKeyEvent->key()==Qt::Key_X)
               && (pKeyEvent->modifiers() & Qt::ControlModifier || pKeyEvent->modifiers() & Qt::AltModifier)) {
            qApp->quit();
            return true;
        }
        else if (pKeyEvent->key()==Qt::Key_Space) {
            m_pOwner->toggleTimer();
            return true;
        }
    }
    // allow all other events
    return false;
}
