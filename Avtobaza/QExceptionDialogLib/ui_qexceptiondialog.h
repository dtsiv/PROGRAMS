/********************************************************************************
** Form generated from reading UI file 'qexceptiondialog.ui'
**
** Created: Thu Aug 25 12:53:55 2016
**      by: Qt User Interface Compiler version 4.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QEXCEPTIONDIALOG_H
#define UI_QEXCEPTIONDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_QExceptionDialog
{
public:
    QGridLayout *gridLayout;
    QPlainTextEdit *plainTextEdit;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QGraphicsView *graphicsView;

    void setupUi(QDialog *QExceptionDialog)
    {
        if (QExceptionDialog->objectName().isEmpty())
            QExceptionDialog->setObjectName(QString::fromUtf8("QExceptionDialog"));
        QExceptionDialog->resize(769, 600);
        gridLayout = new QGridLayout(QExceptionDialog);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        plainTextEdit = new QPlainTextEdit(QExceptionDialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        QFont font;
        font.setPointSize(14);
        plainTextEdit->setFont(font);
        plainTextEdit->setAcceptDrops(false);
        plainTextEdit->setFrameShape(QFrame::StyledPanel);
        plainTextEdit->setFrameShadow(QFrame::Sunken);
        plainTextEdit->setReadOnly(true);
        plainTextEdit->setBackgroundVisible(false);

        gridLayout->addWidget(plainTextEdit, 1, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(QExceptionDialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        gridLayout->addLayout(horizontalLayout, 2, 0, 1, 1);

        graphicsView = new QGraphicsView(QExceptionDialog);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setMinimumSize(QSize(750, 446));
        graphicsView->setFrameShape(QFrame::NoFrame);
        graphicsView->setFrameShadow(QFrame::Plain);
        graphicsView->setLineWidth(0);
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        gridLayout->addWidget(graphicsView, 0, 0, 1, 1);


        retranslateUi(QExceptionDialog);
        QObject::connect(pushButton, SIGNAL(clicked()), QExceptionDialog, SLOT(accept()));

        QMetaObject::connectSlotsByName(QExceptionDialog);
    } // setupUi

    void retranslateUi(QDialog *QExceptionDialog)
    {
        QExceptionDialog->setWindowTitle(QApplication::translate("QExceptionDialog", "Ooops... This is embarassing!", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("QExceptionDialog", "Dismiss", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class QExceptionDialog: public Ui_QExceptionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QEXCEPTIONDIALOG_H
