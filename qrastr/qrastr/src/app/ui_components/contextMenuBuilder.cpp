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
    m_linkedFormsMenu  = new QMenu(tr("Связанные формы"), menuParent);
    m_linkedMacrosMenu = new QMenu(tr("Макрос"),          menuParent);

    // ── Динамические (парентим к this, живут между вызовами меню) ──────
    m_actDesc = new QAction(this);
    connect(m_actDesc, &QAction::triggered,
            this, [this]() { emit sig_colProp(m_currentCol); });

    m_actSum = new QAction(this);
    m_actSum->setEnabled(false);

    m_actDirect = new QAction(tr("Прямой ввод кода"), this);
    m_actDirect->setCheckable(true);
    connect(m_actDirect, &QAction::triggered,
            this, [this]() { emit sig_directCodeToggle(m_currentCol); });

    // ── Строковые операции ──────────────────────────────────────────────
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

    // ── Выравнивание ────────────────────────────────────────────────────
    m_actTmpl = new QAction(tr("Выравнивание: по шаблону"), this);
    m_actData = new QAction(tr("Выравнивание: по данным"),  this);
    connect(m_actTmpl, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByTemplate);
    connect(m_actData, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByData);

    // ── Экспорт / Импорт / Выборка ──────────────────────────────────────
    m_actExport = new QAction(tr("Экспорт CSV"), this);
    m_actImport = new QAction(tr("Импорт CSV"), this);
    m_actSel    = new QAction(tr("Выборка"),    this);
    connect(m_actExport, &QAction::triggered, this, &ContextMenuBuilder::sig_exportCsv);
    connect(m_actImport, &QAction::triggered, this, &ContextMenuBuilder::sig_importCsv);
    // Передаём текущую колонку вместе с сигналом ─ исправляет баг «неверная колонка»
    connect(m_actSel, &QAction::triggered,
            this, [this]() { emit sig_selection(m_currentCol); });

    // ── Условное форматирование ─────────────────────────────────────────
    m_actCF = new QAction(QIcon(":/icons/edit_cond_formats"),
                          tr("Условное форматирование"), this);
    connect(m_actCF, &QAction::triggered,
            this, [this]() { emit sig_condFormatsEdit(m_currentCol); });
}

void ContextMenuBuilder::prepareForHeader(int column, RCol* col, QMenu* menu)
{
    m_currentCol = column;

    // Убираем нежелательные встроенные пункты Qtitan,
    // остальные (сортировка, группировка, …) оставляем.
    removeUnwantedBuiltins(menu);

    // ── Описание колонки ────────────────────────────────────────────────
    if (col) {
        const std::string desc = col->getDesc()
        + " |" + col->getTitle()
            + "| -(" + col->getColName()
            + "), [" + col->getUnit() + "]";
        m_actDesc->setText(QString::fromStdString(desc));
    } else {
        m_actDesc->setText(tr("(нет данных)"));
    }

    // ── Сумма выделенных ────────────────────────────────────────────────
    auto [count, sum] = calcSumSelected();
    m_actSum->setText(
        QString("Сумма: %1   Элементов: %2").arg(sum).arg(count));

    // ── Прямой ввод кода ────────────────────────────────────────────────
    bool showDirect = false;
    if (col) {
        const auto propTT = col->getComPropTT();
        showDirect = (!col->getNameRef().empty() && propTT == enComPropTT::COM_PR_INT)
                     || propTT == enComPropTT::COM_PR_SUPERENUM;
        m_actDirect->setVisible(showDirect);
        if (showDirect)
            m_actDirect->setChecked(col->isDirectCode());
    }
    m_actDirect->setVisible(showDirect);

    // ── Добавляем пункты в меню заголовка ───────────────────────────────
    menu->addSeparator();
    menu->addAction(m_actDesc);
    menu->addAction(m_actSum);
    menu->addSeparator();
    menu->addAction(m_actTmpl);
    menu->addAction(m_actData);
    menu->addSeparator();
    menu->addAction(m_actCF);
    if (showDirect)
        menu->addAction(m_actDirect);
}

void ContextMenuBuilder::prepareForShow(const MenuContext& ctx, QMenu* menu)
{
    m_currentCol = ctx.column;
    // Сносим ВСЕ встроенные пункты Qtitan — для меню ячейки они не нужны.
    // Qtitan владеет объектами, clear() их не удаляет.
    menu->clear();

    // ── Строковые операции ──────────────────────────────────────────────
    menu->addSeparator();
    menu->addAction(m_actInsert);
    menu->addAction(m_actAdd);
    menu->addAction(m_actDuplicate);
    menu->addAction(m_actDelete);
    menu->addAction(m_actGroup);
    menu->addSeparator();

    // ── Экспорт / Импорт / Выборка ──────────────────────────────────────
    menu->addAction(m_actExport);
    menu->addAction(m_actImport);
    menu->addAction(m_actSel);
    menu->addSeparator();

    // ── Связанные подменю ───────────────────────────────────────────────
    rebuildLinkedSubmenus(ctx.row);
    menu->addMenu(m_linkedFormsMenu);
    menu->addMenu(m_linkedMacrosMenu);
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