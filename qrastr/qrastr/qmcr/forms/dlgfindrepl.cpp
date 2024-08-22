#include "dlgfindrepl.h"
#include "ui_dlgfindrepl.h"

DlgFindRepl::DlgFindRepl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgFindRepl)
{
    ui->setupUi(this);
}

DlgFindRepl::~DlgFindRepl()
{
    delete ui;
}

void DlgFindRepl::on_pbFind_clicked(){
    QString qstrFind{"find streing привет"};
    SciHlp::_params_findrepl params_findrepl{qstrFind,qstrFind,true};

    qDebug()<<qstrFind<< " | " <<params_findrepl.qstrFind_ << "\n";
    emit chngFindRepl(params_findrepl);
}

