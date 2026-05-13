#include "filterManager.h"
#include "filter/autoFilter/autoFilterWidget.h"
#include "filter/autoFilter/autoFilterCondition.h"
#include "filter/customFilterCondition.h"
#include "rmodel.h"
#include "tables/selectionDialog.h"

FilterManager::FilterManager(Qtitan::GridTableView* view, RModel* model,
                             QWidget* parentWidget)
    : QObject(parentWidget), m_view(view), m_model(model), m_parentWidget(parentWidget){

    // ── Строка автофильтра (скрыта по умолчанию) ──
    m_autoFilterCond = std::make_unique<AutoFilterCondition>(m_view->filter());
    m_autoFilter     = std::make_unique<AutoFilterWidget>(m_view, nullptr);
    m_autoFilter->setVisible(false);

    // Подписываемся на изменение истории фильтров QTitan.
    // history::changed срабатывает когда встроенный фильтр (FilterDialog,
    // FilterEditor) применяет новое условие через setCondition(..., true).
    connect(m_view->filter()->history(), &Qtitan::GridFilterHistory::changed,
            this, &FilterManager::slot_onBuiltinFilterChanged);

    // Сигналы AutoFilterWidget → applyRule
    connect(m_autoFilter.get(), &AutoFilterWidget::sig_filterChanged,
            this, [this](int colIndex, const FilterRule& rule) {
                applyRule(colIndex, rule);
            });
}

void FilterManager::toggle(bool checked){

    m_autoFilter->setVisible(checked);
    if (!checked) {
        // Сброс условий при скрытии
        m_autoFilterCond->clearAll();
        // clearAll уже испустит filterChanged → rebuildCombinedFilter
        // но для надёжности вызовем явно
        m_autoFilter->clearAll();
        rebuildCombinedFilter();
    }
}

void FilterManager::applyRule(int colIndex, const FilterRule& rule){

    if (rule.isActive())
        m_autoFilterCond->setRule(colIndex, rule);
    else
        m_autoFilterCond->clearRule(colIndex);
    rebuildCombinedFilter();
}

void FilterManager::setSelection(const std::string& selection){

    m_selection       = selection;
    rebuildCombinedFilter();
}

void FilterManager::resetAfterModelReset() {
    // После полного сброса модели очищаем автофильтр:
    // структура колонок могла измениться.
    m_autoFilterCond->clearAll();
    m_autoFilter->clearAll(); // очищает поля и эмитит filterChanged→ rebuild
    m_autoFilter->rebuild(); // пересоздаёт ячейки под новые колонки

    m_selection.clear();
    m_builtinCondition = nullptr;
    rebuildCombinedFilter();
}

void FilterManager::slot_onBuiltinFilterChanged()
{
    // Игнорируем, если это мы сами вызвали setCondition
    if (m_rebuildingFilter)
        return;

    // Удаляем старый клон
    delete m_builtinCondition;
    m_builtinCondition = nullptr;

    auto* current = m_view->filter()->condition();
    if (current && current->conditionCount() > 0)
        m_builtinCondition = current->clone();

    rebuildCombinedFilter();
}

void FilterManager::rebuildCombinedFilter()
{
    const QString selectionFilter = QString::fromStdString(m_selection);
    const bool hasSelection  = !selectionFilter.isEmpty();
    const bool hasAutoFilter = m_autoFilterCond && m_autoFilterCond->hasActiveRules();

    // Сохраняем встроенное условие Qtitan (может быть nullptr)
    Qtitan::GridFilterCondition* builtinCond = nullptr;
    {
        auto* existing = m_view->filter()->condition();
        // Не трогаем собственные условия (group/selection/autofilter) —
        // они пересоздаются каждый раз. Встроенное условие Qtitan —
        // это то, что filter()->condition() возвращает *до* нашего первого вызова.
        // Чтобы не потерять его, храним отдельно:
        builtinCond = m_builtinCondition;
    }

    const bool hasBuiltin = builtinCond && builtinCond->conditionCount() > 0;

    if (!hasSelection && !hasAutoFilter && !hasBuiltin) {
        m_view->filter()->setActive(false);
        return;
    }

    auto* group = new Qtitan::GridFilterGroupCondition(m_view->filter());

    if (hasBuiltin)
        group->addCondition(builtinCond->clone());

    if (hasSelection) {
        // Обращаемся к модели — она сама знает имя таблицы и идёт через RTDA
        const std::vector<long> indices =
            m_model->getRowsBySelection(m_selection);
        auto* selCond = new CustomFilterCondition(m_view->filter());
        for (long idx : indices)
            selCond->addRow(static_cast<int>(idx));
        group->addCondition(selCond);
    }

    if (hasAutoFilter){
        // clone() — QTitan владеет копией, мы сохраняем оригинал для изменений
        group->addCondition(m_autoFilterCond->clone());
    }

    // addToHistory = false — иначе это снова вызовет slot_onBuiltinFilterChanged
    m_view->filter()->setCondition(group, /*addToHistory=*/false);
    m_view->filter()->setActive(true);

    m_rebuildingFilter = false;
}

void FilterManager::slot_openSelection(ModelIndex col)
{
    const auto* prcol = m_model->getRCol(col);
    std::string colName = prcol ? prcol->getColName() : "";

    auto* dlg = new SelectionDialog(m_selection, colName, m_parentWidget);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, &SelectionDialog::sig_selectionAccepted,
            this, &FilterManager::slot_setFiltrForSelection);
    // Закрываем диалог сразу после того, как пользователь принял выборку
    connect(dlg, &SelectionDialog::sig_selectionAccepted,
            dlg, &QDialog::close);

    dlg->show();
}

void FilterManager::slot_setFiltrForSelection(std::string selection){
    setSelection(selection);
}
