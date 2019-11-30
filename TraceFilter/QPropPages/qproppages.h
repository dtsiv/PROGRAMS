#ifndef QPROPPAGES_H
#define QPROPPAGES_H

#include <QtGui>
#include <QtWidgets>
#include <QTabWidget>

#define QPROPPAGES_ACTIVE_TAB                        "ActivePropertyPage"

// color selection button
class QColorSelectionButton : public QPushButton {
    Q_OBJECT

public:
    QColorSelectionButton(const QColor &c, QWidget *parent=0);
    ~QColorSelectionButton();
    QColor& getSelection();

private slots:
    void onClicked();
    void onColorSelected(const QColor &color);

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QColor m_qcColorSelection;
    QColorDialog *m_pcdColorDlg;
};

// dialog with property tabs
class QPropPages : public QDialog {
    Q_OBJECT

public:
    QPropPages(QObject *pOwner, QWidget *parent = 0);
    ~QPropPages();

signals:
    void testPgConnection();

public slots:
    void onAccepted();
    void onSQLiteFileChoose();

    // public-visible interface controls
public:
    QLineEdit *m_pleDBDatabaseName;
    QLineEdit *m_pleDBDatabaseUser;
    QLineEdit *m_pleDBDatabasePassword;
    QLineEdit *m_pleDBDatabaseEncoding;
    QLineEdit *m_pleDBSqliteFileName;
    QLineEdit *m_pleDBDatabaseHostname;
    QLineEdit *m_pleMainCtrl;
    QPushButton *m_pbAccept;
    QRadioButton *m_prbDBEngineSqlite;
    QRadioButton *m_prbDBEnginePostgres;
    QButtonGroup *m_pbgDBEngine;

private:
    QObject *m_pOwner;
    QTabWidget *m_pTabWidget;
};

#endif // QPROPPAGES_H
