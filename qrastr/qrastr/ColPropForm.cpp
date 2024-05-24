#include "ColPropForm.h"
#include "ui_ColPropForm.h"

ColPropForm::ColPropForm(RData* prdata,RCol* prcol,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColPropForm)
{
    std::string title = prdata->t_name_ + "[" +prdata->t_title_ + "]." + prcol->name() + "[" +prcol->title() + "]" ;
    setWindowTitle(title.c_str());
    windowTitle();
    ui->setupUi(this);
    setName(prcol->name().c_str());
    setTitle(prcol->title().c_str());
    setDesc(prcol->desc().c_str());
    setWidth(prcol->width().c_str());
    setPrec(prcol->prec().c_str());
    setExpr(prcol->expr().c_str());

}

ColPropForm::~ColPropForm()
{
    delete ui;
}
//Setter
void ColPropForm::setName(const QString& name)
{
    ui->te_name->appendPlainText(name);
}
void ColPropForm::setTitle(const QString& title)
{
    ui->te_title->appendPlainText(title);
}
void ColPropForm::setDesc(const QString& desc)
{
    ui->te_descr->appendPlainText(desc);
}
void ColPropForm::setWidth(const QString &width)
{
    ui->te_width->appendPlainText(width);
}
void ColPropForm::setPrec(const QString &prec)
{
    ui->te_precision->appendPlainText(prec);
}
void ColPropForm::setExpr(const QString &expr)
{
    ui->te_expression->appendPlainText(expr);
}

//Getter
QString ColPropForm::name() const
{
    return ui->te_name->toPlainText();
}

//Slots
void ColPropForm::on_btn_cancel_clicked()
{
    this->close();
}

void ColPropForm::on_btn_ok_clicked()
{
    //TO DO: need change propertires

    this->close();
}

