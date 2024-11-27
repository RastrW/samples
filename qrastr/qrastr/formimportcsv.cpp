#include "formimportcsv.h"
#include "ui_formimportcsv.h"
#include <QFileDialog>

formimportcsv::formimportcsv(RData* _prdata,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formimportcsv)
{
    ui->setupUi(this);
    prdata_ = _prdata;
    ui->setupUi(this);
    ui->lineEdit_tname->setText(_prdata->t_name_.c_str());
    ui->lineEdit_params->setText(_prdata->get_cols().c_str());
    //ui->lineEdit_viborka->setText(_prdata->

}

formimportcsv::~formimportcsv()
{
    delete ui;
}

void formimportcsv::accept()
{
    std::string table_name =  ui->lineEdit_tname->text().toStdString();
    std::string div =  ui->lineEdit_divider->text().toStdString();
    std::string cols = ui->lineEdit_params->text().toStdString();
    std::string selection = ui->lineEdit_viborka->text().toStdString();

    IRastrTablesPtr tablesx{ prdata_->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(table_name) };

    eCSVCode kod = eCSVCode::Replace;
    if (ui->radioButton_add->isChecked())
        kod = eCSVCode::Add;
    if (ui->radioButton_load->isChecked())
        kod = eCSVCode::Replace;
    if (ui->radioButton_load2->isChecked())
        kod = eCSVCode::Index;
    if (ui->radioButton_union->isChecked())
        kod = eCSVCode::KeyAdd;
    if (ui->radioButton_update->isChecked())
        kod = eCSVCode::Key;

    table->ReadCsv(kod,File,cols,div,"");

    this->close();
}

//Не работает кнопка, хз почему
void formimportcsv::on_pushButton_clicked()
{
    auto qfile = QFileDialog::getOpenFileName(this,
                                                  tr("Сохранение"), "/home", tr(""));
    File = qfile.toStdString();
    ui->lineEdit_file->setText(qfile);
}

void formimportcsv::on_pushButton_2_clicked()
{
    QFileDialog fileDlg( this );
    fileDlg.setAcceptMode( QFileDialog::AcceptSave );
    fileDlg.setWindowTitle( "Сохранение" );
    fileDlg.setDirectory( std::filesystem::current_path() );
    fileDlg.setAcceptMode( QFileDialog::AcceptSave );

    if ( fileDlg.exec() == QDialog::Rejected )
        return;

    File = (fileDlg.selectedFiles())[0].toStdString();
}

