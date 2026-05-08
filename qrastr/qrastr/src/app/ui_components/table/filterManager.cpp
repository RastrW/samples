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
    m_autoFilterCond = new AutoFilterCondition(m_view->filter());
    m_autoFilter     = new AutoFilterWidget(m_view, parentWidget);
    m_autoFilter->setVisible(false);

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

    if (hasSelection) { /* … как раньше … */ }

    if (hasAutoFilter)
        group->addCondition(m_autoFilterCond->clone());

    m_view->filter()->setCondition(group, true);
    m_view->filter()->setActive(true);
}

void FilterManager::slot_openSelection(RDataPos col)
{
    const auto* prcol = m_model->getRCol(col);
    std::string colName = prcol ? prcol->getColName() : "";

    auto* selectionDialog = new SelectionDialog(m_selection, colName, m_parentWidget);
    selectionDialog->setAttribute(Qt::WA_DeleteOnClose);

    connect(selectionDialog, &SelectionDialog::sig_selectionAccepted,
            this, &FilterManager::slot_setFiltrForSelection);

    // Закрываем диалог сразу после того, как пользователь принял выборку
    connect(selectionDialog, &SelectionDialog::sig_selectionAccepted,
            selectionDialog, &QDialog::close);

    selectionDialog->show();
}

void FilterManager::slot_setFiltrForSelection(std::string selection){
    setSelection(selection);
}
