#include "formimportcsv2.h"
#include "ui_formimportcsv2.h"
#include <QFileDialog>

formimportcsv2::formimportcsv2(RData* _prdata,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formimportcsv2)
{
    prdata_ = _prdata;
    ui->setupUi(this);
    ui->lineEdit_tname->setText(_prdata->t_name_.c_str());
    ui->lineEdit_params->setText(_prdata->get_cols().c_str());
}

formimportcsv2::~formimportcsv2()
{
    delete ui;
}

void formimportcsv2::on_pushButton_clicked()
{
    QFileDialog fileDlg( this );
    fileDlg.setAcceptMode( QFileDialog::AcceptOpen );
    fileDlg.setWindowTitle( "Открытие" );
    fileDlg.setDirectory( QString::fromStdString(std::filesystem::current_path().string()) );
    if ( fileDlg.exec() == QDialog::Rejected )
        return;

    //File = (fileDlg.selectedFiles())[0].toStdString();
    ui->lineEdit_file->setText((fileDlg.selectedFiles())[0]);
}


void formimportcsv2::on_buttonBox_accepted()
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

    std::string file =  ui->lineEdit_file->text().toStdString();
    IRastrResultVerify{table->ReadCsv(kod,file,cols,div,"")};

    this->close();
}

