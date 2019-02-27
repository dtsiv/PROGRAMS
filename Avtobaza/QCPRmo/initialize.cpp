#include "stdafx.h"
#include <QtNetwork>
#include <QtGui>

#include "initialize.h"

//******************************************************************************
//
//******************************************************************************
InitializeForm::InitializeForm(QWidget * parent /* = 0 */, 
                               Qt::WFlags flags /* = Qt::WindowStaysOnTopHint 
                                                 [ | Qt::X11BypassWindowManagerHint ]*/ ) 
    : QDialog(parent, flags) {
        setupUi(this);
        setWindowIcon(QIcon(QPixmap(":/Resources/qtdemo.ico")));
}

//******************************************************************************
//
//******************************************************************************
void InitializeForm::onSetLoadStatus(QString qs) {
    QStringList qsl = qs.split(":"); 
    labelMessage -> setText(qsl.at(0));
    if(qsl.count() > 1) {   
        int i = qsl.at(1).toInt();  
        progressBar -> setValue(i);
    }
    update();
    QCoreApplication::instance() -> processEvents();
}



