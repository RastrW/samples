#include "formsettingsforms.h"
#include "ui_formsettingsforms.h"

FormSettingsForms::FormSettingsForms(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSettingsForms)
{
    ui->setupUi(this);
}

FormSettingsForms::~FormSettingsForms()
{
    delete ui;
}
