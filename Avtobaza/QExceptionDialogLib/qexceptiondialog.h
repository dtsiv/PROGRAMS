#ifndef QEXCEPTIONDIALOG_H
#define QEXCEPTIONDIALOG_H

#include <QtCore>
#include <QDialog>

#include "qexceptiondialog_global.h"

namespace Ui {
class QExceptionDialog;
}
class QGraphicsScene;
class QExceptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QExceptionDialog(QString exceptionWhat, QWidget *parent = 0);
    ~QExceptionDialog();

private:
    Ui::QExceptionDialog *ui;
    QGraphicsScene *m_pgsPlane;
};

void QEXCEPTIONDIALOG_EXPORT showExceptionDialog(QString exceptionWhat);

#endif // QEXCEPTIONDIALOG_H

