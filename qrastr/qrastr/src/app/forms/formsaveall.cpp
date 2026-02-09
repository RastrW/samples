#include "formsaveall.h"
#include "ui_formsaveall.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include "utils.h"

formsaveall::formsaveall( QAstra* _pqastra, QMap<QString,QString> _mFilesLoad,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formsaveall)
{

    pqastra =    _pqastra;
    mFilesLoad = _mFilesLoad;
    ui->setupUi(this);
    ui->twSaveFiles->setColumnCount(4);
    ui->twSaveFiles->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twSaveFiles->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->twSaveFiles->horizontalHeader()->setStretchLastSection(true);
    //ui->twSaveFiles->resizeColumnsToContents();
    ui->twSaveFiles->resizeRowsToContents();
    ui->twSaveFiles->verticalHeader()->setVisible(false); // hide row numbers
    QStringList headers;
    headers << "Сохранить" << "Шаблон" << "Файл"<<"путь";
    ui->twSaveFiles->setHorizontalHeaderLabels(headers);
    ui->twSaveFiles->setColumnWidth(static_cast<int>(_cols::save),80);
    ui->twSaveFiles->setColumnWidth(static_cast<int>(_cols::templ),120);
    ui->twSaveFiles->setColumnWidth(static_cast<int>(_cols::file),120);
    ui->twSaveFiles->setColumnWidth(static_cast<int>(_cols::path),400);
    //ui->twSaveFiles->horizontalHeader()->setVisible(false);
}

formsaveall::~formsaveall()
{
    delete ui;
}

void formsaveall::showEvent( QShowEvent* event )
{
    ui->twSaveFiles->setRowCount(0);
    int n_row_num = 0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    for (auto [_shabl,_file] : asKeyValueRange(mFilesLoad) ) {
#else
    for (auto [_shabl,_file] : mFilesLoad.asKeyValueRange() ) {
#endif
        dir_shabl = QFileInfo(_shabl).path();   // assume all templates at one directory
        ui->twSaveFiles->insertRow(n_row_num);
        QTableWidgetItem* ptwi_checkbox = new QTableWidgetItem();
        ptwi_checkbox->data(Qt::CheckStateRole);
        ptwi_checkbox->setCheckState(Qt::Checked);
        ui->twSaveFiles->setItem( n_row_num, static_cast<int>(_cols::save), ptwi_checkbox );

        QTableWidgetItem* ptwi_shabl = new QTableWidgetItem(  QFileInfo(_shabl).fileName());
       // ptwi_shabl->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->twSaveFiles->setItem( n_row_num, static_cast<int>(_cols::templ), ptwi_shabl );

        QTableWidgetItem* ptwi_file = new QTableWidgetItem(    QFileInfo(_file).fileName() );
      //  ptwi_file->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->twSaveFiles->setItem( n_row_num, static_cast<int>(_cols::file), ptwi_file );

        QTableWidgetItem* ptwi_path = new QTableWidgetItem(  QFileInfo(_file).path());
      //  ptwi_path->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->twSaveFiles->setItem( n_row_num, static_cast<int>(_cols::path), ptwi_path );

        n_row_num++;
    }
}

void formsaveall::on_buttonBox_accepted()
{
    QTableWidget *twSaveFiles = ui->twSaveFiles;
    for( int n_rownum = 0; n_rownum < twSaveFiles->rowCount() ; n_rownum++ ){
        const QTableWidgetItem* ptwi_checkbox = twSaveFiles->item( n_rownum, static_cast<int>(_cols::save) );
        if(Qt::Checked == ptwi_checkbox->checkState()){
            const QTableWidgetItem* ptwi_shabl = twSaveFiles->item( n_rownum, static_cast<int>(_cols::templ) );
            const QTableWidgetItem* ptwi_file = twSaveFiles->item( n_rownum, static_cast<int>(_cols::file) );
            const QTableWidgetItem* ptwi_path = twSaveFiles->item( n_rownum, static_cast<int>(_cols::path) );



            QFileInfo qfileinfo;
            qfileinfo.setFile(QDir(ptwi_path->text()),ptwi_file->text());

            QFileInfo qshablinfo;
            qshablinfo.setFile(dir_shabl,ptwi_shabl->text());

            std::string sfile = qfileinfo.absoluteFilePath().toStdString().c_str();
            std::string sshabl;
            if (qshablinfo.isFile() )
                sshabl = qshablinfo.absoluteFilePath().toStdString().c_str();
            else
                sshabl = "";

            pqastra->Save(sfile,sshabl);
        }
    }
}

