#include "formselection.h"
#include "ui_formselection.h"
#include "rtabwidget.h"

FormSelection::FormSelection(std::string _selection ,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FormSelection)
{
    selection = _selection;
    ui->setupUi(this);
    ui->textEdit->setFocus();
    ui->textEdit->setPlainText(  selection.c_str());
}

FormSelection::~FormSelection()
{
    delete ui;
}

void FormSelection::on_buttonBox_accepted()
{
    RtabWidget* pRTW = static_cast<RtabWidget*>(this->parent());
    QString selection = ui->textEdit->toPlainText();
    pRTW->SetSelection(selection.toStdString());

    int a= 1;
}

