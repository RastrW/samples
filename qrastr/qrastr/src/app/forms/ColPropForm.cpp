#include "ColPropForm.h"
#include "ui_ColPropForm.h"
#include "rmodel.h"
#include <QtitanGrid.h>

ColPropForm::ColPropForm(RData* prdata, Qtitan::GridTableView* view,
                         RCol* prcol,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColPropForm),
    m_prdata{prdata},
    m_prcol{prcol},
    m_view{view}{
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
    ///@todo: need change propertires
    IRastrResultVerify{m_prdata->pqastra_->getRastr()->SetLockEvent(true)};
    m_prcol->set_prec(getPrec().toStdString().c_str());

    m_prcol->set_prop(FieldProperties::Description, getDesc().toStdString());
    m_prcol->set_prop(FieldProperties::Expression, getExpr().toStdString());
    m_prcol->set_prop(FieldProperties::Title, getTitle().toStdString());
    m_prcol->set_prop(FieldProperties::Width, getWidth().toStdString());
    long textind = m_prcol->getIndex();
    IRastrResultVerify{m_prdata->pqastra_->getRastr()->SetLockEvent(false)};

    // 1. Получаем колонку с правильным приведением типов
    auto* tableView = static_cast<Qtitan::GridTableView*>(m_view);
    auto* column_base = tableView->getColumn(textind);
    auto* column_qt = static_cast<Qtitan::GridTableColumn*>(column_base);

    // 2. Устанавливаем точность (Decimals)
    if (column_qt) {
        // 3. Вызываем editorRepository()
        Qtitan::GridEditorRepository* repo = column_qt->editorRepository();

        // 4. Приводим репозиторий к числовому типу и устанавливаем точность
        auto* numEditor = qobject_cast<Qtitan::GridNumericEditorRepository*>(repo);
        if (numEditor) {
            numEditor->setDecimals(getPrec().toInt());
        }
    }
    this->close();
}

