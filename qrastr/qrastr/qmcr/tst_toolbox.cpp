#include "tst_toolbox.h"
#include "ui_tst_toolbox.h"

Tst_ToolBox::Tst_ToolBox(QWidget *parent)
    //: QToolBox(parent)
    : QDialog(parent)
    , ui(new Ui::Tst_ToolBox)
{
    ui->setupUi(this);
}

Tst_ToolBox::~Tst_ToolBox()
{
    delete ui;
}
