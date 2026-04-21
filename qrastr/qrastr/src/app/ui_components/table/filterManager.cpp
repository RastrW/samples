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
    // сохраняем для rebuildCombinedFilter
    m_selectionFilter = QString::fromStdString(selection);
    rebuildCombinedFilter();
}

void FilterManager::resetAfterModelReset(){  
    // После полного сброса модели очищаем автофильтр:
    // структура колонок могла измениться.
    m_autoFilterCond->clearAll();
    m_autoFilter->clearAll(); // очищает поля и эмитит filterChanged→ rebuild
    m_autoFilter->rebuild(); // пересоздаёт ячейки под новые колонки

    m_selection.clear();
    m_selectionFilter.clear();
    rebuildCombinedFilter();
}

void FilterManager::rebuildCombinedFilter(){

    const bool hasSelection  = !m_selectionFilter.isEmpty();
    const bool hasAutoFilter = m_autoFilterCond && m_autoFilterCond->hasActiveRules();

    if (!hasSelection && !hasAutoFilter) {
        m_view->filter()->setActive(false);
        return;
    }

    auto* group = new Qtitan::GridFilterGroupCondition(m_view->filter());

    if (hasSelection) {
        // Обращаемся к модели — она сама знает имя таблицы и идёт через RTDM
        const std::vector<long> indices =
            m_model->getRowsBySelection(m_selection);
        auto* selCond = new CustomFilterCondition(m_view->filter());
        for (long idx : indices)
            selCond->addRow(static_cast<int>(idx));
        group->addCondition(selCond);
    }

    if (hasAutoFilter)
        // clone() — QTitan владеет копией, мы сохраняем оригинал для изменений
        group->addCondition(m_autoFilterCond->clone());

    m_view->filter()->setCondition(group, true);
    m_view->filter()->setActive(true);
}

void FilterManager::slot_openSelection(int col)
{
    RCol* prcol = m_model->getRCol(col);
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
