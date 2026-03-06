#include "ColPropForm.h"
#include "rdata.h"
#include <QtitanGrid.h>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include "qastra.h"
#include <astra/IPlainRastrWrappers.h>

ColPropForm::ColPropForm(RData* prdata, Qtitan::GridTableView* view,
                         RCol* prcol, QWidget *parent)
    : QDialog(parent)
    , m_prdata(prdata)
    , m_prcol(prcol)
    , m_view(view){

    std::string title = prdata->t_name_ + "[" + prdata->t_title_ + "]." +
                        prcol->getColName() + "[" + prcol->getTitle() + "]";
    setWindowTitle(QString::fromStdString(title));
    setWindowModality(Qt::ApplicationModal);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setupUi();

    m_leName->setText(QString::fromStdString(prcol->getColName()));
    m_leTitle->setText(QString::fromStdString(prcol->getTitle()));
    m_teDescr->appendPlainText(QString::fromStdString(prcol->getDesc()));
    m_leWidth->setText(QString::fromStdString(prcol->getWidth()));
    m_lePrecision->setText(QString::fromStdString(prcol->getPrec()));
    m_leExpression->setText(QString::fromStdString(prcol->getExpr()));
}

void ColPropForm::setupUi()
{
    m_leName       = new QLineEdit;
    m_leName->setReadOnly(true);
    m_leName->setModified(false);

    m_leTitle      = new QLineEdit;
    m_teDescr      = new QPlainTextEdit;
    m_leWidth      = new QLineEdit;
    m_lePrecision  = new QLineEdit;
    m_leExpression = new QLineEdit;

    QLabel* lWidth = new QLabel(tr("Ширина"));
    QLabel* lPrec = new QLabel(tr("Точность"));

    // --- Кнопки ---
    auto* btn_ok     = new QPushButton(tr("ОК"));
    auto* btn_cancel = new QPushButton(tr("Отмена"));

    auto* rowButtons = new QHBoxLayout;
    rowButtons->addStretch();
    rowButtons->addWidget(btn_ok);
    rowButtons->addWidget(btn_cancel);

    connect(btn_ok,     &QPushButton::clicked, this, &ColPropForm::on_btn_ok_clicked);
    connect(btn_cancel, &QPushButton::clicked, this, &ColPropForm::close);

    // --- Основной layout ---
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(10, 10, 10, 10);
    grid->setVerticalSpacing(10);

    grid->addWidget(new QLabel(tr("Имя")),       0, 0, 1, 1, Qt::AlignVCenter);
    grid->addWidget(m_leName,                    0, 1, 1, 3);

    grid->addWidget(new QLabel(tr("Название")),  1, 0, Qt::AlignVCenter);
    grid->addWidget(m_leTitle,                   1, 1, 1, 3);

    grid->addWidget(new QLabel(tr("Описание")),  2, 0, 1, 1, Qt::AlignTop);
    grid->addWidget(m_teDescr,                   2, 1, 1, 3);

    grid->addWidget(lWidth,              3, 0, 1, 1);
    grid->addWidget(m_leWidth,           3, 1, 1, 1);
    grid->addWidget(lPrec,               3, 2, 1, 1);
    grid->addWidget(m_lePrecision,       3, 3, 1, 1);

    grid->addWidget(new QLabel(tr("Формула")), 4, 0, 1, 1, Qt::AlignVCenter);
    grid->addWidget(m_leExpression,             4, 1, 1, 3);

    grid->addLayout(rowButtons,                5, 2, 1, 2);

    setLayout(grid);
}

void ColPropForm::on_btn_ok_clicked()
{
    IRastrResultVerify{m_prdata->getAstra()->getRastr()->SetLockEvent(true)};

    m_prcol->set_prec(m_prdata->getAstra(),
                      m_lePrecision->text().toStdString().c_str());

    m_prcol->set_prop(m_prdata->getAstra(),
                      FieldProperties::Description,
                      m_teDescr->toPlainText().toStdString());
    m_prcol->set_prop(m_prdata->getAstra(),
                      FieldProperties::Expression,
                      m_leExpression->text().toStdString());
    m_prcol->set_prop(m_prdata->getAstra(),
                      FieldProperties::Title,
                      m_leTitle->text().toStdString());
    m_prcol->set_prop(m_prdata->getAstra(),
                      FieldProperties::Width,
                      m_leWidth->text().toStdString());

    long textind = m_prcol->getIndex();
    IRastrResultVerify{m_prdata->getAstra()->getRastr()->SetLockEvent(false)};
    // 1. Получаем колонку с правильным приведением типов
    auto* tableView   = static_cast<Qtitan::GridTableView*>(m_view);
    auto* column_base = tableView->getColumn(textind);
    auto* column_qt   = static_cast<Qtitan::GridTableColumn*>(column_base);
    // 2. Устанавливаем точность (Decimals)
    if (column_qt) {
        // 3. Вызываем editorRepository()
        Qtitan::GridEditorRepository* repo = column_qt->editorRepository();
        // 4. Приводим репозиторий к числовому типу и устанавливаем точность
        auto* numEditor = qobject_cast<Qtitan::GridNumericEditorRepository*>(repo);
        if (numEditor)
            numEditor->setDecimals(m_lePrecision->text().toInt());
    }

    close();
}
