#include "ColPropForm.h"
#include "ui_ColPropForm.h"
#include "delegatedoubleitem.h"


ColPropForm::ColPropForm(RData* _prdata, RTableView* _ptv, Qtitan::GridTableView* _view, RCol* _prcol,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColPropForm)
{
    ptv = _ptv;
    view = _view;
    prdata = _prdata;

    prcol = _prcol;
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
QString ColPropForm::getName()
{
   return ui->te_name->toPlainText();
}
QString ColPropForm::getTitle()
{
    return ui->te_title->toPlainText();
}
QString ColPropForm::getDesc()
{
    return ui->te_descr->toPlainText();
}
QString ColPropForm::getWidth()
{
    return ui->te_width->toPlainText();
}
QString ColPropForm::getPrec()
{
    return ui->te_precision->toPlainText();
}
QString ColPropForm::getExpr()
{
    return ui->te_expression->toPlainText();
}

//Slots
void ColPropForm::on_btn_cancel_clicked()
{
    this->close();
}

void ColPropForm::on_btn_ok_clicked()
{
    //TO DO: need change propertires

    IRastrResultVerify{prdata->pqastra_->getRastr()->SetLockEvent(true)};
    prcol->set_prec(getPrec().toStdString().c_str());
    //prcol->set_prop(FieldProperties::Name, getName().toStdString());
    prcol->set_prop(FieldProperties::Description, getDesc().toStdString());
    prcol->set_prop(FieldProperties::Expression, getExpr().toStdString());
    prcol->set_prop(FieldProperties::Title, getTitle().toStdString());
    prcol->set_prop(FieldProperties::Width, getWidth().toStdString());

    //static_cast<DelegateDoubleItem*>(ptv->itemDelegateForColumn(prcol->index))->set_prec(prec().toInt());
    IRastrResultVerify{prdata->pqastra_->getRastr()->SetLockEvent(false)};

    Qtitan::GridTableColumn* column_qt = (Qtitan::GridTableColumn *)view->getColumn(prcol->index);
    ((Qtitan::GridNumericEditorRepository *)column_qt->editorRepository())->setDecimals(getPrec().toInt());

    this->close();
}

