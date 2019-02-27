#include <QDialog>
#include <QGraphicsScene>
#include <QApplication>
#include <QDesktopWidget>

#include "qexceptiondialog.h"
#include "ui_qexceptiondialog.h"

void showExceptionDialog(QString exceptionWhat) {
    QExceptionDialog w(exceptionWhat);
    w.exec();
}

QExceptionDialog::QExceptionDialog(QString exceptionWhat, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QExceptionDialog),
    m_pgsPlane(0)
{
    ui->setupUi(this);
    QPixmap qpmPlane(":/Resources/plane_small.png");
    m_pgsPlane = new QGraphicsScene(qpmPlane.rect());
    m_pgsPlane->addPixmap(qpmPlane);
    ui->graphicsView->setScene(m_pgsPlane);
    ui->plainTextEdit->setPlainText(exceptionWhat);

    QDesktopWidget* pwgt = QApplication::desktop();
    QRect rectDesktop = pwgt->screenGeometry();
    QRect rectDialog = this->frameGeometry();
    this->move((rectDesktop.width()-rectDialog.width())/2,
               (rectDesktop.height()-rectDialog.height())/2);
    Qt::WFlags flags = Qt::WindowStaysOnTopHint
    #ifdef __linux
        | Qt::X11BypassWindowManagerHint
    #endif
    ;
    this -> setWindowFlags(flags);
}

QExceptionDialog::~QExceptionDialog()
{
    delete ui;
    delete m_pgsPlane;
}
