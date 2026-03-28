#include "contextMenuBuilder.h"
#include <QtitanGrid.h>
#include "rcol.h"
#include "linkedformcontroller.h"
#include <QElapsedTimer>

ContextMenuBuilder::ContextMenuBuilder(Qtitan::GridTableView* view,
                                       LinkedFormController*  linkedFormCtrl,
                                       QObject*               parent)
    : QObject(parent)
    , m_view(view)
    , m_linkedFormCtrl(linkedFormCtrl)
{}

void ContextMenuBuilder::removeUnwantedBuiltins(QMenu* menu)
{
    /// Перебираем копию списка, т.к. removeAction меняет оригинал
    /// @note Горячие клавиши Qtitan блокиру.тся в RtabWidget::eventFilter
    const QList<QAction*> actions = menu->actions();
    for (QAction* act : actions) {
        const QString text = act->text();
        if (text.contains("Подогнать", Qt::CaseInsensitive) ||
            text.contains("Удалить", Qt::CaseInsensitive)) {

            menu->removeAction(act);
            act->setShortcut(QKeySequence()); // снимаем с action
            // Qtitan владеет объектом
        }       
    }
}

void ContextMenuBuilder::initMenu(QWidget* menuParent)
{
    // Подменю связанных форм/макросов парентим к menuParent (виджет-хозяин),
    // чтобы они жили независимо от конкретного QMenu Qtitan
    m_linkedFormsMenu  = new QMenu(tr("Связанные формы"),  menuParent);
    m_linkedMacrosMenu = new QMenu(tr("Макрос"),           menuParent);

    // Все статические экшны парентим к this (ContextMenuBuilder),
    // чтобы они пережили любой отдельный QMenu и не удалялись вместе с ним
    m_actDesc = new QAction(this);
    connect(m_actDesc, &QAction::triggered,
            this, [this]() { emit sig_colProp(m_currentCol); });

    m_actSum = new QAction(this);
    m_actSum->setEnabled(false);

    m_actInsert    = new QAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),  tr("Вставить"),            this);
    m_actAdd       = new QAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"),  tr("Добавить"),            this);
    m_actDuplicate = new QAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"), tr("Дублировать"),         this);
    m_actDelete    = new QAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),  tr("Удалить"),             this);
    m_actGroup     = new QAction(QIcon(":/images/column_edit.png"),               tr("Групповая коррекция"), this);

    m_actInsert   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    m_actAdd      ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    m_actDuplicate->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    m_actDelete   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));

    connect(m_actInsert,    &QAction::triggered, this, &ContextMenuBuilder::sig_insertRow);
    connect(m_actAdd,       &QAction::triggered, this, &ContextMenuBuilder::sig_addRow);
    connect(m_actDuplicate, &QAction::triggered, this, &ContextMenuBuilder::sig_duplicateRow);
    connect(m_actDelete,    &QAction::triggered, this, &ContextMenuBuilder::sig_deleteRow);
    connect(m_actGroup,     &QAction::triggered, this, &ContextMenuBuilder::sig_groupCorrection);

    m_actTmpl = new QAction(tr("Выравнивание: по шаблону"), this);
    m_actData = new QAction(tr("Выравнивание: по данным"),  this);
    connect(m_actTmpl, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByTemplate);
    connect(m_actData, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByData);

    m_actExport = new QAction(tr("Экспорт CSV"), this);
    m_actImport = new QAction(tr("Импорт CSV"), this);
    m_actSel    = new QAction(tr("Выборка"),    this);
    connect(m_actExport, &QAction::triggered, this, &ContextMenuBuilder::sig_exportCsv);
    connect(m_actImport, &QAction::triggered, this, &ContextMenuBuilder::sig_importCsv);
    connect(m_actSel,    &QAction::triggered, this, &ContextMenuBuilder::sig_selection);

    m_actDirect = new QAction(tr("Прямой ввод кода"), this);
    m_actDirect->setCheckable(true);
    connect(m_actDirect, &QAction::triggered,
            this, [this]() { emit sig_directCodeToggle(m_currentCol); });

    m_actCF = new QAction(QIcon(":/icons/edit_cond_formats"),
                          tr("Условное форматирование"), this);
    connect(m_actCF, &QAction::triggered,
            this, [this]() { emit sig_condFormatsEdit(m_currentCol); });
}

void ContextMenuBuilder::prepareForShow(const MenuContext& ctx, QMenu* menu)
{
    m_currentCol = ctx.column;

    // ── 1. Убираем нежелательные встроенные пункты Qtitan ────────────────
    removeUnwantedBuiltins(menu);

    // ── 2. Обновляем динамический текст ──────────────────────────────────
    const std::string desc = ctx.col->getDesc()
                             + " |" + ctx.col->getTitle()
                             + "| -(" + ctx.col->getColName()
                             + "), [" + ctx.col->getUnit() + "]";
    m_actDesc->setText(QString::fromStdString(desc));

    auto [count, sum] = calcSumSelected();
    m_actSum->setText(
        QString("Сумма: %1   Элементов: %2").arg(sum).arg(count));

    // ── 3. Прямой ввод кода ───────────────────────────────────────────────
    const auto propTT = ctx.col->getComPropTT();
    const bool showDirect =
        (!ctx.col->getNameRef().empty() && propTT == enComPropTT::COM_PR_INT)
        || propTT == enComPropTT::COM_PR_SUPERENUM;
    m_actDirect->setVisible(showDirect);
    if (showDirect)
        m_actDirect->setChecked(ctx.col->isDirectCode());

    // ── 4. Вставляем custom пункты в Qtitan-меню ───────────────────────────
    menu->addSeparator();
    menu->addAction(m_actDesc);
    menu->addAction(m_actSum);
    menu->addSeparator();
    menu->addAction(m_actInsert);
    menu->addAction(m_actAdd);
    menu->addAction(m_actDuplicate);
    menu->addAction(m_actDelete);
    menu->addAction(m_actGroup);
    menu->addSeparator();
    menu->addAction(m_actTmpl);
    menu->addAction(m_actData);
    menu->addSeparator();
    menu->addAction(m_actExport);
    menu->addAction(m_actImport);
    menu->addAction(m_actSel);
    menu->addAction(m_actDirect);

    // ── 5. Связанные подменю (очищаем и перестраиваем) ───────────────────
    rebuildLinkedSubmenus(ctx.row);
    menu->addMenu(m_linkedFormsMenu);
    menu->addMenu(m_linkedMacrosMenu);

    menu->addAction(m_actCF);
}

void ContextMenuBuilder::rebuildLinkedSubmenus(int contextRow)
{
    // Очищаем только действия внутри подменю — сами QMenu остаются в дереве
    m_linkedFormsMenu ->clear();
    m_linkedMacrosMenu->clear();

    m_linkedFormCtrl->buildLinkedFormsMenu(contextRow,  m_linkedFormsMenu);
    m_linkedFormCtrl->buildLinkedMacroMenu(contextRow,  m_linkedMacrosMenu);
}

std::tuple<int, double> ContextMenuBuilder::calcSumSelected() const
{
    QModelIndexList selected = m_view->selection()->selectedIndexes();
    if (selected.empty()) return {0, 0.0};

    int    count = 0;
    double total = 0.0;
    for (const QModelIndex& idx : selected) {
        bool ok;
        double v = idx.data().toDouble(&ok);
        if (ok) { total += v; ++count; }
    }
    return {count, total};
}

void ContextMenuBuilder::prepareForHeader(int column, QMenu* menu)
{
    menu->clear();   // убираем встроенные пункты QTitan

    // Заглушка — действия добавите позже
}