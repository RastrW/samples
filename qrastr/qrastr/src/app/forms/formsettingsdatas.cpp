#include "formsettingsdatas.h"
#include "ui_formsettingsdatas.h"

FormSettingsDatas::FormSettingsDatas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSettingsDatas)
{
    ui->setupUi(this);
}

FormSettingsDatas::~FormSettingsDatas()
{
    delete ui;
}
