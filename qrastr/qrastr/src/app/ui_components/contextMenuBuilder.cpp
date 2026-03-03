#include "contextMenuBuilder.h"
#include <QtitanGrid.h>
#include "rcol.h"
#include "linkedformcontroller.h"

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

void ContextMenuBuilder::populate(QMenu* menu, const MenuContext& ctx)
{
    // ── 1. Убираем нежелательные встроенные пункты ────────────────────────
    removeUnwantedBuiltins(menu);

    // ── 2. Информация о колонке ────────────────────────────────────────────
    {
        std::string desc = ctx.col->desc()
                         + " |" + ctx.col->title()
                         + "| -(" + ctx.col->name()
                         + "), [" + ctx.col->unit() + "]";
        menu->addSeparator();
        // Захватываем col в лямбду по значению индекса, не по указателю —
        // указатель ctx.col действителен только на время populate()
        const int capturedCol = ctx.column;
        auto* actDesc = new QAction(QString::fromStdString(desc), menu); // parent=menu
        connect(actDesc, &QAction::triggered,
                this, [this, capturedCol]() { emit sig_colProp(capturedCol); });
        menu->addAction(actDesc);
    }

    // ── 3. Сумма выделенных ячеек ─────────────────────────────────────────
    {
        auto [count, sum] = calcSumSelected();
        auto* actSum = new QAction(                                  // parent=menu
            QString("Сумма: %1   Элементов: %2").arg(sum).arg(count),
            menu);
        actSum->setEnabled(false); // информационный пункт, не кликабельный
        menu->addAction(actSum);
    }

    menu->addSeparator();

    // ── 4. Операции со строками ───────────────────────────────────────────
    // Все QAction создаются с parent=menu → автоудаляются вместе с меню.
    // Шорткаты здесь только визуальные подсказки; реальные шорткаты
    // регистрируются в RtabWidget::setupShortcuts()
    {
        auto* actInsert    = new QAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),
                                         tr("Вставить"),         menu);
        auto* actAdd       = new QAction(QIcon(":/images/Rastr3_grid_addrow_16x16.png"),
                                         tr("Добавить"),         menu);
        auto* actDuplicate = new QAction(QIcon(":/images/Rastr3_grid_duprow_16x161.png"),
                                         tr("Дублировать"),      menu);
        auto* actDelete    = new QAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),
                                         tr("Удалить"),          menu);
        auto* actGroup     = new QAction(QIcon(":/images/column_edit.png"),
                                         tr("Групповая коррекция"), menu);

        actInsert   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
        actAdd      ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
        actDuplicate->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
        actDelete   ->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));

        connect(actInsert,    &QAction::triggered, this, &ContextMenuBuilder::sig_insertRow);
        connect(actAdd,       &QAction::triggered, this, &ContextMenuBuilder::sig_addRow);
        connect(actDuplicate, &QAction::triggered, this, &ContextMenuBuilder::sig_duplicateRow);
        connect(actDelete,    &QAction::triggered, this, &ContextMenuBuilder::sig_deleteRow);
        connect(actGroup,     &QAction::triggered, this, &ContextMenuBuilder::sig_groupCorrection);

        menu->addAction(actInsert);
        menu->addAction(actAdd);
        menu->addAction(actDuplicate);
        menu->addAction(actDelete);
        menu->addAction(actGroup);
    }

    menu->addSeparator();

    // ── 5. Выравнивание ───────────────────────────────────────────────────
    {
        auto* actTmpl = new QAction(tr("Выравнивание: по шаблону"), menu);
        auto* actData = new QAction(tr("Выравнивание: по данным"),  menu);
        connect(actTmpl, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByTemplate);
        connect(actData, &QAction::triggered, this, &ContextMenuBuilder::sig_widthByData);
        menu->addAction(actTmpl);
        menu->addAction(actData);
    }

    menu->addSeparator();

    // ── 6. CSV / Выборка ──────────────────────────────────────────────────
    {
        auto* actExport = new QAction(tr("Экспорт CSV"), menu);
        auto* actImport = new QAction(tr("Импорт CSV"), menu);
        auto* actSel    = new QAction(tr("Выборка"),    menu);
        connect(actExport, &QAction::triggered, this, &ContextMenuBuilder::sig_exportCsv);
        connect(actImport, &QAction::triggered, this, &ContextMenuBuilder::sig_importCsv);
        connect(actSel,    &QAction::triggered, this, &ContextMenuBuilder::sig_selection);
        menu->addAction(actExport);
        menu->addAction(actImport);
        menu->addAction(actSel);
    }

    // ── 7. Прямой ввод кода ─────────────────────
    {
        const auto propTT = ctx.col->getComPropTT();
        if ((!ctx.col->getNameRef().empty() && propTT == enComPropTT::COM_PR_INT)
            || propTT == enComPropTT::COM_PR_SUPERENUM)
        {
            const int capturedCol = ctx.column;
            auto* actDirect = new QAction(tr("Прямой ввод кода"), menu); // parent=menu
            actDirect->setCheckable(true);
            actDirect->setChecked(ctx.col->isDirectCode());
            connect(actDirect, &QAction::triggered,
                    this, [this, capturedCol]() {
                        emit sig_directCodeToggle(capturedCol);
                    });
            menu->addAction(actDirect);
        }
    }

    // ── 8. Связанные формы и макросы ──────────────────────────────────────
    // buildLinkedFormsMenu / buildLinkedMacroMenu возвращают QMenu* с parent=menu;
    // addMenu передаёт ownership меню в родительское меню.
    menu->addMenu(m_linkedFormCtrl->buildLinkedFormsMenu(ctx.row));
    menu->addMenu(m_linkedFormCtrl->buildLinkedMacroMenu(ctx.row));

    // ── 9. Условное форматирование ────────────────────────────────────────
    {
        const int capturedCol = ctx.column;
        auto* actCF = new QAction(
            QIcon(":/icons/edit_cond_formats"),
            tr("Условное форматирование"), menu);               // parent=menu
        connect(actCF, &QAction::triggered,
                this, [this, capturedCol]() {
                    emit sig_condFormatsEdit(capturedCol);
                });
        menu->addAction(actCF);
    }
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
