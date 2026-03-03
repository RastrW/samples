#include "formselection.h"
#include "ui_formselection.h"
#include "rtabwidget.h"

FormSelection::FormSelection(std::string colName,
                             QWidget *parent)
    : QDialog(parent),
    ui(new Ui::FormSelection),
    m_colName{colName}
{
    ui->setupUi(this);
    ui->textEdit->setFocus();
    std::string hint;
    if (!colName.empty()){
        hint = m_colName + "=";
    }else{
        hint = m_colName;
    }
    ui->textEdit->setPlainText(  hint.c_str());
}

FormSelection::~FormSelection()
{
    delete ui;
}

void FormSelection::on_buttonBox_accepted()
{
    QString selection = ui->textEdit->toPlainText();
    emit sig_selectionAccepted(selection.toStdString());
}

