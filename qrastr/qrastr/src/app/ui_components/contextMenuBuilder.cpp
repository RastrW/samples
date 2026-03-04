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
    m_menu = new QMenu(menuParent);
    // ── Убираем нежелательные встроенные пункты ────────────────────────
    removeUnwantedBuiltins(m_menu);

    // ── Описание колонки ─────────────────────────────────────────────────
    m_menu->addSeparator();
    m_actDesc = new QAction(m_menu);
    connect(m_actDesc, &QAction::triggered,
            this, [this]() { emit sig_colProp(m_currentCol); });
    m_menu->addAction(m_actDesc);

    // ── Сумма выделенных ─────────────────────────────────────────────────
    m_actSum = new QAction(m_menu);
    m_actSum->setEnabled(false);
    m_menu->addAction(m_actSum);

    m_menu->addSeparator();

    // ── Строковые операции (статика) ─────────────────────────────────────
    auto* actInsert    = new QAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),
                                  tr("Вставить"),            m_menu);
    auto* actAdd       = new QAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"),
                               tr("Добавить"),            m_menu);
    auto* actDuplicate = new QAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"),
                                     tr("Дублировать"),         m_menu);
    auto* actDelete    = new QAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),
                                  tr("Удалить"),             m_menu);
    auto* actGroup     = new QAction(QIcon(":/images/column_edit.png"),
                                 tr("Групповая коррекция"), m_menu);

    actInsert   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    actAdd      ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    actDuplicate->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    actDelete   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));

    connect(actInsert,    &QAction::triggered, this, &ContextMenuBuilder::sig_insertRow);
    connect(actAdd,       &QAction::triggered, this, &ContextMenuBuilder::sig_addRow);
    connect(actDuplicate, &QAction::triggered, this, &ContextMenuBuilder::sig_duplicateRow);
    connect(actDelete,    &QAction::triggered, this, &ContextMenuBuilder::sig_deleteRow);
    connect(actGroup,     &QAction::triggered, this, &ContextMenuBuilder::sig_groupCorrection);

    m_menu->addAction(actInsert);
    m_menu->addAction(actAdd);
    m_menu->addAction(actDuplicate);
    m_menu->addAction(actDelete);
    m_menu->addAction(actGroup);

    m_menu->addSeparator();

    // ── Выравнивание (статика) ───────────────────────────────────────────
    auto* actTmpl = new QAction(tr("Выравнивание: по шаблону"), m_menu);
    auto* actData = new QAction(tr("Выравнивание: по данным"),  m_menu);
    connect(actTmpl, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByTemplate);
    connect(actData, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByData);
    m_menu->addAction(actTmpl);
    m_menu->addAction(actData);

    m_menu->addSeparator();

    // ── CSV / Выборка (статика) ──────────────────────────────────────────
    auto* actExport = new QAction(tr("Экспорт CSV"), m_menu);
    auto* actImport = new QAction(tr("Импорт CSV"), m_menu);
    auto* actSel    = new QAction(tr("Выборка"),    m_menu);
    connect(actExport, &QAction::triggered, this, &ContextMenuBuilder::sig_exportCsv);
    connect(actImport, &QAction::triggered, this, &ContextMenuBuilder::sig_importCsv);
    connect(actSel,    &QAction::triggered, this, &ContextMenuBuilder::sig_selection);
    m_menu->addAction(actExport);
    m_menu->addAction(actImport);
    m_menu->addAction(actSel);

    // ── Прямой ввод кода (видимость меняется, пункт постоянный) ─────────
    m_actDirect = new QAction(tr("Прямой ввод кода"), m_menu);
    m_actDirect->setCheckable(true);
    connect(m_actDirect, &QAction::triggered,
            this, [this]() { emit sig_directCodeToggle(m_currentCol); });
    m_menu->addAction(m_actDirect);

    // ── Подменю: связанные формы и макросы ──────────────────────────────
    // Создаём пустые подменю — заполняются в rebuildLinkedSubmenus()
    m_linkedFormsMenu  = new QMenu(tr("Связанные формы"),  m_menu);
    m_linkedMacrosMenu = new QMenu(tr("Макрос"),           m_menu);
    m_menu->addMenu(m_linkedFormsMenu);
    m_menu->addMenu(m_linkedMacrosMenu);

    // ── Условное форматирование (статика) ────────────────────────────────
    auto* actCF = new QAction(QIcon(":/icons/edit_cond_formats"),
                              tr("Условное форматирование"), m_menu);
    connect(actCF, &QAction::triggered,
            this, [this]() { emit sig_condFormatsEdit(m_currentCol); });
    m_menu->addAction(actCF);
}

QMenu* ContextMenuBuilder::prepareForShow(const MenuContext& ctx)
{
    QElapsedTimer totalTimer;
    totalTimer.start();

    // ── 0. Установка текущей колонки ─────────────────────────────────────
    QElapsedTimer stepTimer;
    stepTimer.start();

    m_currentCol = ctx.column;

    qInfo() << "[prepareForShow] Step 0 (set current column):"
            << stepTimer.elapsed() << "ms";

    // ── 1. Текст описания колонки ─────────────────────────────────────────
    stepTimer.restart();

    const std::string desc = ctx.col->getDesc()
                             + " |" + ctx.col->getTitle()
                             + "| -(" + ctx.col->getColName()
                             + "), [" + ctx.col->getUnit() + "]";
    m_actDesc->setText(QString::fromStdString(desc));

    qInfo() << "[prepareForShow] Step 1 (build description text):"
            << stepTimer.elapsed() << "ms";

    // ── 2. Сумма выделенных ───────────────────────────────────────────────
    stepTimer.restart();

    auto [count, sum] = calcSumSelected();
    m_actSum->setText(
        QString("Сумма: %1   Элементов: %2").arg(sum).arg(count));

    qInfo() << "[prepareForShow] Step 2 (calculate selected sum):"
            << stepTimer.elapsed() << "ms";

    // ── 3. Прямой ввод кода ───────────────────────────────────────────────
    stepTimer.restart();

    const auto propTT = ctx.col->getComPropTT();
    const bool showDirect =
        (!ctx.col->getNameRef().empty() && propTT == enComPropTT::COM_PR_INT)
        || propTT == enComPropTT::COM_PR_SUPERENUM;

    m_actDirect->setVisible(showDirect);
    if (showDirect)
        m_actDirect->setChecked(ctx.col->isDirectCode());

    qInfo() << "[prepareForShow] Step 3 (direct code handling):"
            << stepTimer.elapsed() << "ms";

    // ── 4. Связанные подменю ──────────────────────────────────────────────
    stepTimer.restart();

    rebuildLinkedSubmenus(ctx.row);

    qInfo() << "[prepareForShow] Step 4 (rebuild linked submenus):"
            << stepTimer.elapsed() << "ms";

    // ── Итоговое время ────────────────────────────────────────────────────
    qInfo() << "[prepareForShow] TOTAL:"
            << totalTimer.elapsed() << "ms";

    return m_menu;
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
