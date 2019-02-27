#include "stdafx.h"
#include <QObject>
#include <QDialog>
#include "ui_initialize.h"
//*****************************************************************************
//
//*****************************************************************************
class InitializeForm : public QDialog, public Ui::InitializeForm {
Q_OBJECT

public:
    InitializeForm(QWidget * parent = 0, 
                   Qt::WFlags flags = Qt::WindowStaysOnTopHint
                                #ifdef __linux
                                    | Qt::X11BypassWindowManagerHint
                                #endif
                  );

public slots:
    void onSetLoadStatus(QString qs);
};
