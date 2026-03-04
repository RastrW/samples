#include "formgroupcorrection.h"
#include "ui_formgroupcorrection.h"

formgroupcorrection::formgroupcorrection(RData* _prdata,RCol* _prcol, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::formgroupcorrection)
{
    prdata_ = _prdata;
    prcol_ = _prcol;
    ui->setupUi(this);
    QString str_lab3 = "в таблице ";
    str_lab3.append(_prdata->t_name_.c_str());
    ui->label_3->setText(str_lab3);

    std::string str_par = _prdata->t_name_;
    str_par.append("[").append(_prdata->t_title_).append("].");  //node[Узлы].

    for ( auto &col : *prdata_)
    {
        std::string par = str_par;
        par.append(col.getColName()).append("[").append(col.getTitle()).append("]");
        ui->comboBox->addItem(par.c_str());
    }
    ui->comboBox->setCurrentIndex(prcol_->getIndex());
}

formgroupcorrection::~formgroupcorrection()
{
    delete ui;
}

void formgroupcorrection::on_buttonBox_accepted()
{
    selection_ = ui->lineEdit_selection->text().toStdString();
    expression_ = ui->lineEdit_expression->text().toStdString();
    prcol_->calc(prdata_->getAstra(), expression_,selection_);
    this->close();
}

