#include "tst2_dialog.h"
#include "ui_tst2_dialog.h"

Tst2_Dialog::Tst2_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tst2_Dialog)
{
    ui->setupUi(this);
}

Tst2_Dialog::~Tst2_Dialog()
{
    delete ui;
}
