#include "stdafx.h"
#include "targetselection.h"
#include "ui_targetselection.h"

TargetSelection::TargetSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TargetSelection) {
    ui->setupUi(this);
    dbHost = DATABASE_HOST;
    dbConnect();

    filterModel = new QSortFilterProxyModel(parent);
    filterModel->setSourceModel(&model);

    model.setQuery("select * from targets order by id asc",db);
    model.setHeaderData( model.record().indexOf("freqFromHB"),Qt::Horizontal,QString::fromLocal8Bit("Начальная частота ВЧ (МГц)"));
    model.setHeaderData( model.record().indexOf("freqToHB"),Qt::Horizontal,QString::fromLocal8Bit("Конечная частота ВЧ (МГц)"));


    ui->tableView->setModel(filterModel);
    ui->tableView->setColumnHidden(model.record().indexOf("id"),true);
    ui->tableView->setColumnHidden(model.record().indexOf("img"),true);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->setSortingEnabled(true);

    QObject::connect(ui->tableView,SIGNAL(clicked(const QModelIndex &)),SLOT(onViewClicked(const QModelIndex &)));

    QHeaderView *headerView = ui->tableView->horizontalHeader();

    /*
    QObject::connect(headerView,
        SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
        SLOT(onSortByColumn(int,Qt::SortOrder))
    );
    */

    ui->tableView->show();
}

void TargetSelection::dbConnect() {
    db = QSqlDatabase::database(DATABASE_CONN);
    // qDebug() << "db.isValid()=" << db.isValid();
    if (!db.isValid()) db = QSqlDatabase::addDatabase(DATABASE_TYPE,DATABASE_CONN);
    db.setDatabaseName(DATABASE_NAME);
    db.setUserName(DATABASE_ROLE);
    db.setHostName(dbHost);
    db.setPassword(DATABASE_PASS);
    if (!db.open()) {
        qDebug() << db.lastError().text();
        QMessageBox::warning(this,"Connection error",db.lastError().text());
    }
}

TargetSelection::~TargetSelection() {
    db.close();
    delete ui;
}

void TargetSelection::onSortByColumn(int column, Qt::SortOrder order) {
    // ui->tableView->sortByColumn(column, order);
}

void TargetSelection::onViewClicked(const QModelIndex & idx) {
    //int row=idx.row(), col_id=model.record().indexOf("id");
    int row=idx.row();
    // qlonglong id = model.data(idx.sibling(row,model.record().indexOf("id"))).toLongLong();
    QString img = model.data(idx.sibling(row,model.record().indexOf("img"))).toString();
    QPalette pal;
    QPixmap pm(ui->scrollArea->size());
    pm.loadFromData(QByteArray::fromBase64(img.toLatin1()));
    pm = pm.scaled(ui->scrollArea->size(),Qt::KeepAspectRatio);
    pal.setBrush(ui->saContents->backgroundRole(), QBrush(pm));
    ui->saContents->setPalette(pal);
    ui->saContents->setFixedSize(pm.width(),pm.height());
    ui->saContents->setAutoFillBackground(true);
    this->freqFromHB=model.data(idx.sibling(row,model.record().indexOf("freqfromhb"))).toInt();
    this->freqToHB=model.data(idx.sibling(row,model.record().indexOf("freqtohb"))).toInt();
}
