#include "formexportcsv.h"
#include "ui_formexportcsv.h"
#include <QFileDialog>

formexportcsv::formexportcsv(RData* _prdata,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formexportcsv)
{
    prdata_ = _prdata;
    ui->setupUi(this);
    ui->lineEdit_2->setText(_prdata->t_name_.c_str());
    ui->lineEdit_3->setText(_prdata->get_cols().c_str());

}

formexportcsv::~formexportcsv()
{
    delete ui;
}

void formexportcsv::on_pushButton_clicked()
{
    QFileDialog fileDlg( this );
    fileDlg.setAcceptMode( QFileDialog::AcceptSave );
    fileDlg.setWindowTitle( "Сохранение" );
    fileDlg.setDirectory( std::filesystem::current_path() );
    fileDlg.setAcceptMode( QFileDialog::AcceptSave );

    if ( fileDlg.exec() == QDialog::Rejected )
        return;

    File = (fileDlg.selectedFiles())[0].toStdString();

    //auto file1Name = QFileDialog::getOpenFileName(this,
    //                                         tr("Сохранение"), "/home", tr(""));
    ui->lineEdit->setText(File.c_str());
}

void formexportcsv::accept()
{
    std::string table_name =  ui->lineEdit_2->text().toStdString();
    std::string div =  ui->lineEdit_4->text().toStdString();
    std::string cols = ui->lineEdit_3->text().toStdString();
    std::string selection = ui->lineEdit_4->text().toStdString();


    IRastrTablesPtr tablesx{ prdata_->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(table_name) };
    if (!selection.empty())
        IRastrResultVerify{table->SetSelection(selection)};

    eCSVCode kod = eCSVCode::Replace;
    if (ui->radioButton->isChecked())
        kod = eCSVCode::Replace;
    if (ui->radioButton_2->isChecked())
        kod = eCSVCode::KeyAdd;
    if (ui->radioButton_2->isChecked())
        kod = eCSVCode::ReplaceName;

    IRastrResultVerify{table->ToCsv(kod,File,cols,div)};

    this->close();
}



void formexportcsv::on_formexportcsv_accepted()
{

}

