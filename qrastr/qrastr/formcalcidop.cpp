#include "formcalcidop.h"
#include "ui_formcalcidop.h"

formcalcidop::formcalcidop(QAstra* _pqastra,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formcalcidop)
{
    ui->setupUi(this);
    pqastra = _pqastra;

}

formcalcidop::~formcalcidop()
{
    delete ui;
}

void formcalcidop::on_buttonBox_accepted()
{
    double T = ui->lineEdit->text().toDouble();
    double AV = ui->lineEdit_2->text().toDouble();
    std::string sel = ui->lineEdit_3->text().toStdString();

    pqastra->CalcIdop(T,AV,sel);
}


void formcalcidop::on_checkBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if (ui->checkBox->isChecked())
    {
        ui->label_4->setEnabled(true);
        ui->dateTimeEdit->setEnabled((true));
    }
    else
    {
        ui->label_4->setEnabled(false);
        ui->dateTimeEdit->setEnabled((false));
    }
}

