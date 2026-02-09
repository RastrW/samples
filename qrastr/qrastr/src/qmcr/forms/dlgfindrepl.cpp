#include <QDebug>
#include "dlgfindrepl.h"
#include "ui_dlgfindrepl.h"

DlgFindRepl::DlgFindRepl(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgFindRepl){
    ui->setupUi(this);
}
DlgFindRepl::~DlgFindRepl(){
    delete ui;
}
void DlgFindRepl::on_pbFind_clicked(){
    const QString qstrFind{ ui->leFind->text() };
    SciHlp::_params_find params_find{ qstrFind };
    qDebug()<<qstrFind<< " | " <<params_find.qstrFind_ << "\n";
    emit chngFind(params_find);
}
