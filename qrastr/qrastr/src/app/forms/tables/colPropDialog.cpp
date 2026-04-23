#include "colPropDialog.h"
#include <QtitanGrid.h>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>

ColPropDialog::ColPropDialog(ITableRepository*                   repo,
                             const ITableRepository::ColumnSchema& schema,
                             Qtitan::GridTableView*              view,
                             QWidget*                            parent)
    : QDialog(parent)
    , m_repo(repo)
    , m_schema(schema)
    , m_view(view)
{
    std::string title = schema.tableName + "." +
                        schema.name + "[" + schema.title + "]";
    setWindowTitle(QString::fromStdString(title));
    setWindowModality(Qt::ApplicationModal);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setupUi();

    m_leName->setText(QString::fromStdString(schema.name));
    m_leTitle->setText(QString::fromStdString(schema.title));
    m_teDescr->appendPlainText(QString::fromStdString(schema.description));
    m_leWidth->setText(QString::fromStdString(schema.width));
    m_lePrecision->setText(QString::fromStdString(schema.prec));
    m_leExpression->setText(QString::fromStdString(schema.expression));
}

void ColPropDialog::setupUi()
{
    m_leName       = new QLineEdit;
    m_leName->setReadOnly(true);
    m_leName->setModified(false);
    m_leName->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: gray; }");
    m_leName->setFocusPolicy(Qt::NoFocus);

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

    connect(btn_ok,     &QPushButton::clicked, this, &ColPropDialog::on_btn_ok_clicked);
    connect(btn_cancel, &QPushButton::clicked, this, &ColPropDialog::close);

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

void ColPropDialog::on_btn_ok_clicked()
{
    m_repo->setLockEvent(true);

    m_repo->setColumnProperty(m_schema.tableName, m_schema.name,
                              FieldProperties::Precision,
                              m_lePrecision->text().toStdString());
    m_repo->setColumnProperty(m_schema.tableName, m_schema.name,
                              FieldProperties::Description,
                              m_teDescr->toPlainText().toStdString());
    m_repo->setColumnProperty(m_schema.tableName, m_schema.name,
                              FieldProperties::Expression,
                              m_leExpression->text().toStdString());
    m_repo->setColumnProperty(m_schema.tableName, m_schema.name,
                              FieldProperties::Title,
                              m_leTitle->text().toStdString());
    m_repo->setColumnProperty(m_schema.tableName, m_schema.name,
                              FieldProperties::Width,
                              m_leWidth->text().toStdString());
    m_repo->setLockEvent(false);

    // Обновляем decimals в редакторе колонки QTitan
    auto* column_qt = static_cast<Qtitan::GridTableColumn*>(
        m_view->getColumn(m_schema.index));
    // Устанавливаем точность (Decimals)
    if (column_qt) {
        auto* numEditor = qobject_cast<Qtitan::GridNumericEditorRepository*>(
            column_qt->editorRepository());
        if (numEditor)
            numEditor->setDecimals(m_lePrecision->text().toInt());
    }

    close();
}
