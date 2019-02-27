#ifndef TARGETSELECTION_H
#define TARGETSELECTION_H

#include <QDialog>
#include <QtSql>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#define DATABASE_HOST "localhost"
#define DATABASE_NAME "rmo"
#define DATABASE_ROLE "rmo"
#define DATABASE_PASS "123"
#define DATABASE_TYPE "QPSQL"
#define DATABASE_CONN "rmo"

namespace Ui {
class TargetSelection;
}

class TargetSelection : public QDialog
{
    Q_OBJECT

public:
    explicit TargetSelection(QWidget *parent = 0);
    void dbConnect();
    ~TargetSelection();

    int freqFromHB;
    int freqToHB;

public slots:
    void onViewClicked(const QModelIndex &);
    void onSortByColumn(int, Qt::SortOrder);

private:
    Ui::TargetSelection *ui;
    QSqlDatabase db;
    QSqlQueryModel model;
    QSortFilterProxyModel *filterModel;
    QString dbHost;
};

#endif // TARGETSELECTION_H
